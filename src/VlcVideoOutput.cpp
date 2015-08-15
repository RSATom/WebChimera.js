#include "VlcVideoOutput.h"

#include <cassert>

///////////////////////////////////////////////////////////////////////////////
VlcVideoOutput::VideoFrame::VideoFrame() :
    _width( 0 ), _height( 0 ), _size( 0 ), _frameBuffer( nullptr )
{
}

void VlcVideoOutput::VideoFrame::video_cleanup_cb()
{
    if( !_tmpFrameBuffer.empty() )
        std::vector<char>().swap( _tmpFrameBuffer );
}

///////////////////////////////////////////////////////////////////////////////
unsigned VlcVideoOutput::RV32VideoFrame::video_format_cb( char* chroma,
                                                          unsigned* width, unsigned* height,
                                                          unsigned* pitches, unsigned* lines )
{
    _width = *width;
    _height = *height;

    memcpy( chroma, vlc::DEF_CHROMA, sizeof( vlc::DEF_CHROMA ) - 1 );
    *pitches = *width * vlc::DEF_PIXEL_BYTES;
    *lines = *height;

    _size = *pitches * *lines;
    _tmpFrameBuffer.resize( _size );

    return 1;
}

void* VlcVideoOutput::RV32VideoFrame::video_lock_cb( void** planes )
{
    void* buffer;
    if( _tmpFrameBuffer.empty() ) {
        buffer = _frameBuffer;
    } else {
        if( _frameBuffer ) {
            std::vector<char>().swap( _tmpFrameBuffer );
            buffer = _frameBuffer;
        } else {
            buffer = _tmpFrameBuffer.data();
        }
    }

    *planes = buffer;

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
VlcVideoOutput::I420VideoFrame::I420VideoFrame() :
    _uPlaneOffset( 0 ), _vPlaneOffset( 0 )
{
}

unsigned VlcVideoOutput::I420VideoFrame::video_format_cb( char* chroma,
                                                          unsigned* width, unsigned* height,
                                                          unsigned* pitches, unsigned* lines )
{
    _width = *width;
    _height = *height;

    const char CHROMA[] = "I420";

    memcpy( chroma, CHROMA, sizeof( CHROMA ) - 1 );

    const unsigned evenWidth = *width + ( *width & 1 );
    const unsigned evenHeight = *height + ( *height & 1 );

    pitches[0] = evenWidth; if( pitches[0] % 4 ) pitches[0] += 4 - pitches[0] % 4;
    pitches[1] = evenWidth / 2; if( pitches[1] % 4 ) pitches[1] += 4 - pitches[1] % 4;
    pitches[2] = pitches[1];

    assert( 0 == pitches[0] % 4 && 0 == pitches[1] % 4 && 0 == pitches[2] % 4 );

    lines[0] = evenHeight;
    lines[1] = evenHeight / 2;
    lines[2] = lines[1];

    _uPlaneOffset = pitches[0] * lines[0];
    _vPlaneOffset = _uPlaneOffset + pitches[1] * lines[1];

    _size = pitches[0] * lines[0] +
            pitches[1] * lines[1] +
            pitches[2] * lines[2];
    _tmpFrameBuffer.resize( _size );

    return 3;
}

void* VlcVideoOutput::I420VideoFrame::video_lock_cb( void** planes )
{
    char* buffer;
    if( _tmpFrameBuffer.empty() ) {
        buffer = static_cast<char*>( _frameBuffer );
    } else {
        if( _frameBuffer ) {
            std::vector<char>().swap( _tmpFrameBuffer );
            buffer = static_cast<char*>( _frameBuffer );
        } else {
            buffer = _tmpFrameBuffer.data();
        }
    }

    planes[0] = buffer;
    planes[1] = buffer + _uPlaneOffset;
    planes[2] = buffer + _vPlaneOffset;

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
struct VlcVideoOutput::VideoEvent
{
    virtual void process( VlcVideoOutput* ) = 0;
};

///////////////////////////////////////////////////////////////////////////////
struct VlcVideoOutput::RV32FrameSetupEvent : public VlcVideoOutput::VideoEvent
{
    RV32FrameSetupEvent( const std::shared_ptr<RV32VideoFrame>& videoFrame ) :
        _videoFrame( videoFrame ) {}

    void process( VlcVideoOutput* ) override;

    std::weak_ptr<RV32VideoFrame> _videoFrame;
};

void VlcVideoOutput::RV32FrameSetupEvent::process( VlcVideoOutput* videoOutput )
{
    std::shared_ptr<RV32VideoFrame> videoFrame = _videoFrame.lock();

    if( !videoFrame )
        return;

    void* buffer = videoOutput->onFrameSetup( *videoFrame );
    if( buffer )
        videoFrame->setFrameBuffer( buffer );
}

///////////////////////////////////////////////////////////////////////////////
struct VlcVideoOutput::I420FrameSetupEvent : public VlcVideoOutput::VideoEvent
{
    I420FrameSetupEvent( const std::shared_ptr<I420VideoFrame>& videoFrame ) :
        _videoFrame( videoFrame ) {}

    void process( VlcVideoOutput* ) override;

    std::weak_ptr<I420VideoFrame> _videoFrame;
};

void VlcVideoOutput::I420FrameSetupEvent::process( VlcVideoOutput* videoOutput )
{
    std::shared_ptr<I420VideoFrame> videoFrame = _videoFrame.lock();

    if( !videoFrame )
        return;

    void* buffer = videoOutput->onFrameSetup( *videoFrame );
    if( buffer )
        videoFrame->setFrameBuffer( buffer );
}

///////////////////////////////////////////////////////////////////////////////
struct VlcVideoOutput::FrameReadyEvent : public VlcVideoOutput::VideoEvent
{
    void process( VlcVideoOutput* ) override;
};

void VlcVideoOutput::FrameReadyEvent::process( VlcVideoOutput* videoOutput )
{
    if( videoOutput->_waitingFrame.test_and_set() ) //FIXME! use memory_order
        return;

    videoOutput->onFrameReady();
}

///////////////////////////////////////////////////////////////////////////////
struct VlcVideoOutput::FrameCleanupEvent : public VlcVideoOutput::VideoEvent
{
    void process( VlcVideoOutput* ) override;
};

void VlcVideoOutput::FrameCleanupEvent::process( VlcVideoOutput* videoOutput )
{
    videoOutput->onFrameCleanup();
}

///////////////////////////////////////////////////////////////////////////////
VlcVideoOutput::VlcVideoOutput()
{
    uv_loop_t* loop = uv_default_loop();

    uv_async_init( loop, &_async,
        [] ( uv_async_t* handle ) {
            if( handle->data )
                reinterpret_cast<VlcVideoOutput*>( handle->data )->handleAsync();
        }
    );
    _async.data = this;

    _waitingFrame.test_and_set(); //FIXME! use memory_order
}

VlcVideoOutput::~VlcVideoOutput()
{
    uv_close( reinterpret_cast<uv_handle_t*>( &_async ), 0 );
    _async.data = nullptr;
}

unsigned VlcVideoOutput::video_format_cb( char* chroma,
                                          unsigned* width, unsigned* height,
                                          unsigned* pitches, unsigned* lines )
{
    std::unique_ptr<VideoEvent> frameSetupEvent;
    switch( _pixelFormat ) {
        case PixelFormat::RV32: {
            std::shared_ptr<RV32VideoFrame> videoFrame( new RV32VideoFrame() );
            frameSetupEvent.reset( new RV32FrameSetupEvent( videoFrame ) );
            _videoFrame = videoFrame;
            break;
        }
        case PixelFormat::I420:
        default: {
            std::shared_ptr<I420VideoFrame> videoFrame( new I420VideoFrame() );
            frameSetupEvent.reset( new I420FrameSetupEvent( videoFrame ) );
            _videoFrame = videoFrame;
            break;
        }
    }

    const unsigned planeCount = _videoFrame->video_format_cb( chroma,
                                                              width, height,
                                                              pitches, lines );

    _guard.lock();
    _videoEvents.push_back( std::move( frameSetupEvent ) );
    _guard.unlock();
    uv_async_send( &_async );

    return planeCount;
}

void VlcVideoOutput::video_cleanup_cb()
{
    _videoFrame->video_cleanup_cb();

    _guard.lock();
    _videoEvents.emplace_back( new FrameCleanupEvent );
    _guard.unlock();
    uv_async_send( &_async );
}

void* VlcVideoOutput::video_lock_cb( void** planes )
{
    return _videoFrame->video_lock_cb( planes );
}

void VlcVideoOutput::video_unlock_cb( void* picture, void *const * planes )
{
    _videoFrame->video_unlock_cb( picture, planes );
}

void VlcVideoOutput::video_display_cb( void* picture )
{
    if( _videoFrame->video_display_cb( picture ) ) {
        _waitingFrame.clear(); //FIXME! use memory_order

        _guard.lock();
        _videoEvents.emplace_back( new FrameReadyEvent );
        _guard.unlock();
        uv_async_send( &_async );
    }
}

void VlcVideoOutput::handleAsync()
{
    while( !_videoEvents.empty() ) {
        std::deque<std::unique_ptr<VideoEvent> > tmpEvents;
        _guard.lock();
        _videoEvents.swap( tmpEvents );
        _guard.unlock();
        for( const auto& i: tmpEvents ) {
            i->process( this );
        }
    }
}

bool VlcVideoOutput::isFrameReady()
{
    return !_waitingFrame.test_and_set(); //FIXME! use memory_order
}

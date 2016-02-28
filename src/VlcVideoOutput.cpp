#include "VlcVideoOutput.h"

#include <string.h>

#include <cassert>

///////////////////////////////////////////////////////////////////////////////
VlcVideoOutput::VideoFrame::VideoFrame() :
    _width( 0 ), _height( 0 ), _size( 0 ),
    _frameBuffer( nullptr )
{
}

VlcVideoOutput::VideoFrame::~VideoFrame()
{
}

void VlcVideoOutput::VideoFrame::waitBuffer()
{
    std::unique_lock<std::mutex> lock( _guard );
    while( !_frameBuffer )
        _waiter.wait( lock );
}

void VlcVideoOutput::VideoFrame::setFrameBuffer( void* frameBuffer )
{
    std::unique_lock<std::mutex> lock( _guard );
    _frameBuffer = frameBuffer;
    _waiter.notify_one();
}

///////////////////////////////////////////////////////////////////////////////
bool VlcVideoOutput::RV32VideoFrame::setup( vmem2_video_format_t* format )
{
    format->chroma = RV32_FOURCC;
    format->plane_count = 1;

    _width = format->visible_width;
    _height = format->visible_height;

    format->pitches[0] = _width * 4;
    format->lines[0] = _height;

    _size = format->pitches[0] * format->lines[0];

    return true;
}

bool VlcVideoOutput::RV32VideoFrame::lock( vmem2_planes_t* planes )
{
    planes->planes[0] = _frameBuffer;

    return true;
}

void VlcVideoOutput::RV32VideoFrame::fillBlack()
{
    if( _frameBuffer )
        memset( _frameBuffer, 0, size() );
}

///////////////////////////////////////////////////////////////////////////////
VlcVideoOutput::I420VideoFrame::I420VideoFrame() :
    _uPlaneOffset( 0 ), _vPlaneOffset( 0 )
{
}

bool VlcVideoOutput::I420VideoFrame::setup( vmem2_video_format_t* format )
{
    format->chroma = I420_FOURCC;
    format->plane_count = 3;

    _width = format->visible_width;
    _height = format->visible_height;

    format->pitches[0] = format->width;
    format->pitches[1] = format->width >> 1;
    format->pitches[2] = format->width >> 1;

    format->lines[0] = format->height;
    format->lines[1] = format->height >> 1;
    format->lines[2] = format->height >> 1;

    _uPlaneOffset = format->pitches[0] * format->lines[0];
    _vPlaneOffset = _uPlaneOffset + format->pitches[1] * format->lines[1];

    _size = format->pitches[0] * format->lines[0] +
            format->pitches[1] * format->lines[1] +
            format->pitches[2] * format->lines[2];

    return true;
}

bool VlcVideoOutput::I420VideoFrame::lock( vmem2_planes_t* planes )
{
    char* buffer= static_cast<char*>( _frameBuffer );

    planes->planes[0] = buffer;
    planes->planes[1] = buffer + _uPlaneOffset;
    planes->planes[2] = buffer + _vPlaneOffset;

    return true;
}

void VlcVideoOutput::I420VideoFrame::fillBlack()
{
    if( _frameBuffer ) {
        char* buffer = static_cast<char*>( _frameBuffer );
        memset( buffer, 0x0, _uPlaneOffset );
        memset( buffer + _uPlaneOffset, 0x80, _vPlaneOffset - _uPlaneOffset );
        memset( buffer + _vPlaneOffset, 0x80, size() - _vPlaneOffset );
    }
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

    videoOutput->_currentVideoFrame = videoFrame;

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

    videoOutput->_currentVideoFrame = videoFrame;

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
    if( videoOutput->_currentVideoFrame ) {
        videoOutput->onFrameCleanup();
        videoOutput->_currentVideoFrame.reset();
    }
}

///////////////////////////////////////////////////////////////////////////////
VlcVideoOutput::VlcVideoOutput() :
    _pixelFormat( PixelFormat::I420 )
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

bool VlcVideoOutput::vmem2_setup( void* opaque, vmem2_video_format_t* format )
{
    return reinterpret_cast<VlcVideoOutput*>( opaque )->setup( format );
}

bool VlcVideoOutput::vmem2_lock( void* opaque, vmem2_planes_t* planes )
{
    return reinterpret_cast<VlcVideoOutput*>( opaque )->lock( planes );
}

void VlcVideoOutput::vmem2_unlock( void* opaque, const vmem2_planes_t* planes )
{
    reinterpret_cast<VlcVideoOutput*>( opaque )->unlock( planes );
}

void VlcVideoOutput::vmem2_display( void* opaque, const vmem2_planes_t* planes )
{
    reinterpret_cast<VlcVideoOutput*>( opaque )->display( planes );
}

void VlcVideoOutput::vmem2_cleanup( void* opaque )
{
    reinterpret_cast<VlcVideoOutput*>( opaque )->cleanup();
}

bool VlcVideoOutput::open( vlc::basic_player* player )
{
    if( !player || !player->is_open() )
        return false;

    vmem2_set_callbacks( player->get_mp(),
                         vmem2_setup ,
                         vmem2_lock, vmem2_unlock, vmem2_display,
                         vmem2_cleanup, this );

    return true;
}

void VlcVideoOutput::close()
{
}

bool VlcVideoOutput::setup( vmem2_video_format_t* format )
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

    _videoFrame->setup( format );

    _guard.lock();
    _videoEvents.push_back( std::move( frameSetupEvent ) );
    _guard.unlock();
    uv_async_send( &_async );

    _videoFrame->waitBuffer();

    return true;
}

void VlcVideoOutput::cleanup()
{
    _guard.lock();
    _videoEvents.emplace_back( new FrameCleanupEvent );
    _guard.unlock();
    uv_async_send( &_async );
}

bool VlcVideoOutput::lock( vmem2_planes_t* planes )
{
    return _videoFrame->lock( planes );
}

void VlcVideoOutput::unlock( const vmem2_planes_t* /*planes*/ )
{
}

void VlcVideoOutput::display( const vmem2_planes_t* /*planes*/ )
{
    notifyFrameReady();
}

void VlcVideoOutput::notifyFrameReady()
{
    _waitingFrame.clear(); //FIXME! use memory_order

    _guard.lock();
    _videoEvents.emplace_back( new FrameReadyEvent );
    _guard.unlock();
    uv_async_send( &_async );
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

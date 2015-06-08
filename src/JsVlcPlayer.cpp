#include "JsVlcPlayer.h"

#include <string.h>

#include "JsVlcPlaylist.h"

v8::Persistent<v8::Function> JsVlcPlayer::_jsConstructor;
std::set<JsVlcPlayer*> JsVlcPlayer::_instances;

///////////////////////////////////////////////////////////////////////////////
class JsVlcPlayer::VideoFrame :
    public std::enable_shared_from_this<JsVlcPlayer::VideoFrame>
{
protected:
    VideoFrame() :
        _frameBuffer( nullptr ) {}

public:
    virtual unsigned video_format_cb( char* chroma,
                                      unsigned* width, unsigned* height,
                                      unsigned* pitches, unsigned* lines,
                                      std::unique_ptr<AsyncData>* asyncData ) = 0;
    virtual void* video_lock_cb( void** planes ) = 0;
    void video_cleanup_cb();

    void setFrameBuffer( char* frameBuffer )
        { _frameBuffer = frameBuffer; }

protected:
    char* _frameBuffer;
    std::vector<char> _tmpFrameBuffer;
};

void JsVlcPlayer::VideoFrame::video_cleanup_cb()
{
    if( !_tmpFrameBuffer.empty() )
        std::vector<char>().swap( _tmpFrameBuffer );
}

///////////////////////////////////////////////////////////////////////////////
class JsVlcPlayer::RV32VideoFrame : public JsVlcPlayer::VideoFrame
{
public:
    unsigned video_format_cb( char* chroma,
                              unsigned* width, unsigned* height,
                              unsigned* pitches, unsigned* lines,
                              std::unique_ptr<AsyncData>* asyncData ) override;
    void* video_lock_cb( void** planes ) override;
};

///////////////////////////////////////////////////////////////////////////////
class JsVlcPlayer::I420VideoFrame : public JsVlcPlayer::VideoFrame
{
public:
    unsigned video_format_cb( char* chroma,
                              unsigned* width, unsigned* height,
                              unsigned* pitches, unsigned* lines,
                              std::unique_ptr<AsyncData>* asyncData ) override;
    void* video_lock_cb( void** planes ) override;

protected:
    unsigned _uPlaneOffset;
    unsigned _vPlaneOffset;
};

///////////////////////////////////////////////////////////////////////////////
struct JsVlcPlayer::AsyncData
{
    virtual void process( JsVlcPlayer* ) = 0;
};

///////////////////////////////////////////////////////////////////////////////
struct JsVlcPlayer::RV32FrameSetupData : public JsVlcPlayer::AsyncData
{
    RV32FrameSetupData( unsigned width, unsigned height, unsigned size,
                        const std::weak_ptr<RV32VideoFrame>& videoFrame ) :
        width( width ), height( height ), size( size ), videoFrame( videoFrame ) {}

    void process( JsVlcPlayer* ) override;

    const unsigned width;
    const unsigned height;
    const unsigned size;

    std::weak_ptr<RV32VideoFrame> videoFrame;
};

void JsVlcPlayer::RV32FrameSetupData::process( JsVlcPlayer* jsPlayer )
{
    jsPlayer->setupBuffer( *this );
}

///////////////////////////////////////////////////////////////////////////////
struct JsVlcPlayer::I420FrameSetupData : public JsVlcPlayer::AsyncData
{
    I420FrameSetupData( unsigned width, unsigned height,
                        unsigned uPlaneOffset, unsigned vPlaneOffset,
                        unsigned size,
                        const std::weak_ptr<I420VideoFrame>& videoFrame ) :
        width( width ), height( height ),
        uPlaneOffset( uPlaneOffset ), vPlaneOffset( vPlaneOffset ),
        size( size ), videoFrame( videoFrame ) {}

    void process( JsVlcPlayer* ) override;

    const unsigned width;
    const unsigned height;
    const unsigned uPlaneOffset;
    const unsigned vPlaneOffset;
    const unsigned size;

    std::weak_ptr<I420VideoFrame> videoFrame;
};

void JsVlcPlayer::I420FrameSetupData::process( JsVlcPlayer* jsPlayer )
{
    jsPlayer->setupBuffer( *this );
}

///////////////////////////////////////////////////////////////////////////////
struct JsVlcPlayer::FrameUpdated : public JsVlcPlayer::AsyncData
{
    void process( JsVlcPlayer* ) override;
};

void JsVlcPlayer::FrameUpdated::process( JsVlcPlayer* jsPlayer )
{
    jsPlayer->frameUpdated();
}

///////////////////////////////////////////////////////////////////////////////
struct JsVlcPlayer::CallbackData : public JsVlcPlayer::AsyncData
{
    CallbackData( JsVlcPlayer::Callbacks_e callback ) :
        callback( callback ) {}

    void process( JsVlcPlayer* );

    const JsVlcPlayer::Callbacks_e callback;
};

void JsVlcPlayer::CallbackData::process( JsVlcPlayer* jsPlayer )
{
    jsPlayer->callCallback( callback );
}

///////////////////////////////////////////////////////////////////////////////
struct JsVlcPlayer::LibvlcEvent : public JsVlcPlayer::AsyncData
{
    LibvlcEvent( const libvlc_event_t& libvlcEvent ) :
        libvlcEvent( libvlcEvent ) {}

    void process( JsVlcPlayer* );

    const libvlc_event_t libvlcEvent;
};

void JsVlcPlayer::LibvlcEvent::process( JsVlcPlayer* jsPlayer )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Callbacks_e callback = CB_Max;

    std::initializer_list<v8::Local<v8::Value> > list;

    switch( libvlcEvent.type ) {
        case libvlc_MediaPlayerMediaChanged:
            callback = CB_MediaPlayerMediaChanged;
            break;
        case libvlc_MediaPlayerNothingSpecial:
            callback = CB_MediaPlayerNothingSpecial;
            break;
        case libvlc_MediaPlayerOpening:
            callback = CB_MediaPlayerOpening;
            break;
        case libvlc_MediaPlayerBuffering: {
            callback = CB_MediaPlayerBuffering;
            list = { Number::New( isolate, libvlcEvent.u.media_player_buffering.new_cache ) };
            break;
        }
        case libvlc_MediaPlayerPlaying:
            callback = CB_MediaPlayerPlaying;
            break;
        case libvlc_MediaPlayerPaused:
            callback = CB_MediaPlayerPaused;
            break;
        case libvlc_MediaPlayerStopped:
            callback = CB_MediaPlayerStopped;
            break;
        case libvlc_MediaPlayerForward:
            callback = CB_MediaPlayerForward;
            break;
        case libvlc_MediaPlayerBackward:
            callback = CB_MediaPlayerBackward;
            break;
        case libvlc_MediaPlayerEndReached:
            callback = CB_MediaPlayerEndReached;
            break;
        case libvlc_MediaPlayerEncounteredError:
            callback = CB_MediaPlayerEncounteredError;
            break;
        case libvlc_MediaPlayerTimeChanged: {
            callback = CB_MediaPlayerTimeChanged;
            const double new_time =
                static_cast<double>( libvlcEvent.u.media_player_time_changed.new_time );
            list = { Number::New( isolate, static_cast<double>( new_time ) ) };
            break;
        }
        case libvlc_MediaPlayerPositionChanged: {
            callback = CB_MediaPlayerPositionChanged;
            list = { Number::New( isolate, libvlcEvent.u.media_player_position_changed.new_position ) };
            break;
        }
        case libvlc_MediaPlayerSeekableChanged: {
            callback = CB_MediaPlayerSeekableChanged;
            list = { Boolean::New( isolate, libvlcEvent.u.media_player_seekable_changed.new_seekable != 0 ) };
            break;
        }
        case libvlc_MediaPlayerPausableChanged: {
            callback = CB_MediaPlayerPausableChanged;
            list = { Boolean::New( isolate, libvlcEvent.u.media_player_pausable_changed.new_pausable != 0 ) };
            break;
        }
        case libvlc_MediaPlayerLengthChanged: {
            callback = CB_MediaPlayerLengthChanged;
            const double new_length =
                static_cast<double>( libvlcEvent.u.media_player_length_changed.new_length );
            list = { Number::New( isolate, new_length ) };
            break;
        }
    }

    if( callback != CB_Max ) {
        jsPlayer->callCallback( callback, list );
    }
}

///////////////////////////////////////////////////////////////////////////////
unsigned JsVlcPlayer::RV32VideoFrame::video_format_cb( char* chroma,
                                                       unsigned* width, unsigned* height,
                                                       unsigned* pitches, unsigned* lines,
                                                       std::unique_ptr<AsyncData>* asyncData  )
{
    memcpy( chroma, vlc::DEF_CHROMA, sizeof( vlc::DEF_CHROMA ) - 1 );
    *pitches = *width * vlc::DEF_PIXEL_BYTES;
    *lines = *height;

    _tmpFrameBuffer.resize( *pitches * *lines );

    if( asyncData ) {
        asyncData->reset(
            new RV32FrameSetupData( *width, *height,
                                    _tmpFrameBuffer.size(),
                                    std::static_pointer_cast<RV32VideoFrame>( shared_from_this() ) ) );
    }

    return 1;
}

void* JsVlcPlayer::RV32VideoFrame::video_lock_cb( void** planes )
{
    char* buffer;
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
unsigned JsVlcPlayer::I420VideoFrame::video_format_cb( char* chroma,
                                                       unsigned* width, unsigned* height,
                                                       unsigned* pitches, unsigned* lines,
                                                       std::unique_ptr<AsyncData>* asyncData )
{
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

    _tmpFrameBuffer.resize( pitches[0] * lines[0] +
                            pitches[1] * lines[1] +
                            pitches[2] * lines[2] );

    if( asyncData ) {
        asyncData->reset(
            new I420FrameSetupData( *width, *height,
                                    _uPlaneOffset,
                                    _vPlaneOffset,
                                    _tmpFrameBuffer.size(),
                                     std::static_pointer_cast<I420VideoFrame>( shared_from_this() ) ) );
    }

    return 3;
}

void* JsVlcPlayer::I420VideoFrame::video_lock_cb( void** planes )
{
    char* buffer;
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

    planes[0] = buffer;
    planes[1] = buffer + _uPlaneOffset;
    planes[2] = buffer + _vPlaneOffset;

    return nullptr;
}

///////////////////////////////////////////////////////////////////////////////
void JsVlcPlayer::closeAll()
{
    for( JsVlcPlayer* p : _instances ) {
        p->close();
    }
}

JsVlcPlayer::JsVlcPlayer( const v8::Local<v8::Array>& vlcOpts ) :
    _libvlc( nullptr ), _pixelFormat( PixelFormat::I420 )
{
    _instances.insert( this );

    initLibvlc( vlcOpts );

    if( _libvlc && _player.open( _libvlc ) ) {
        _player.register_callback( this );
        vlc::basic_vmem_wrapper::open( &_player.basic_player() );
    } else {
        assert( false );
    }

    _jsPlaylist = JsVlcPlaylist::create( *this );

    uv_loop_t* loop = uv_default_loop();

    uv_async_init( loop, &_async,
        [] ( uv_async_t* handle ) {
            if( handle->data )
                reinterpret_cast<JsVlcPlayer*>( handle->data )->handleAsync();
        }
    );
    _async.data = this;
}

void JsVlcPlayer::initLibvlc( const v8::Local<v8::Array>& vlcOpts )
{
    using namespace v8;

    if( _libvlc ) {
        assert( false );
        libvlc_release( _libvlc );
        _libvlc = nullptr;
    }

    if( vlcOpts.IsEmpty() || vlcOpts->Length() == 0 ) {
        _libvlc = libvlc_new( 0, nullptr );
    } else {
        std::deque<std::string> opts;
        std::vector<const char*> libvlcOpts;

        for( unsigned i = 0 ; i < vlcOpts->Length(); ++i ) {
            String::Utf8Value opt( vlcOpts->Get(i)->ToString() );
            if( opt.length() ) {
                auto it = opts.emplace( opts.end(), *opt );
                libvlcOpts.push_back( it->c_str() );
            }
        }

        _libvlc = libvlc_new( libvlcOpts.size(), libvlcOpts.data() );
    }
}

JsVlcPlayer::~JsVlcPlayer()
{
    close();

    _instances.erase( this );
}

void JsVlcPlayer::close()
{
    _player.stop();

    _player.unregister_callback( this );
    vlc::basic_vmem_wrapper::close();

    _player.close();

    _async.data = nullptr;
    uv_close( reinterpret_cast<uv_handle_t*>( &_async ), 0 );

    if( _libvlc ) {
        libvlc_release( _libvlc );
        _libvlc = nullptr;
    }
}

unsigned JsVlcPlayer::video_format_cb( char* chroma,
                                       unsigned* width, unsigned* height,
                                       unsigned* pitches, unsigned* lines )
{
    switch( _pixelFormat ) {
        case PixelFormat::RV32:
            _videoFrame.reset( new RV32VideoFrame() );
            break;
        case PixelFormat::I420:
            _videoFrame.reset( new I420VideoFrame() );
            break;
    }

    std::unique_ptr<AsyncData> asyncData;
    unsigned planeCount = _videoFrame->video_format_cb( chroma,
                                                        width, height,
                                                        pitches, lines,
                                                        &asyncData );
    if( asyncData ) {
        _asyncDataGuard.lock();
        _asyncData.emplace_back( std::move( asyncData ) );
        _asyncDataGuard.unlock();

        uv_async_send( &_async );
    }
    return planeCount;
}

void JsVlcPlayer::video_cleanup_cb()
{
    _videoFrame->video_cleanup_cb();

    _asyncDataGuard.lock();
    _asyncData.emplace_back( new CallbackData( CB_FrameCleanup ) );
    _asyncDataGuard.unlock();

    uv_async_send( &_async );
}

void* JsVlcPlayer::video_lock_cb( void** planes )
{
    return _videoFrame->video_lock_cb( planes );
}

void JsVlcPlayer::video_unlock_cb( void* /*picture*/, void *const * /*planes*/ )
{
}

void JsVlcPlayer::video_display_cb( void* /*picture*/ )
{
    _asyncDataGuard.lock();
    _asyncData.emplace_back( new FrameUpdated() );
    _asyncDataGuard.unlock();
    uv_async_send( &_async );
}

void JsVlcPlayer::media_player_event( const libvlc_event_t* e )
{
    _asyncDataGuard.lock();
    _asyncData.emplace_back( new LibvlcEvent( *e ) );
    _asyncDataGuard.unlock();
    uv_async_send( &_async );
}

void JsVlcPlayer::handleAsync()
{
    while( !_asyncData.empty() ) {
        std::deque<std::unique_ptr<AsyncData> > tmpData;
        _asyncDataGuard.lock();
        _asyncData.swap( tmpData );
        _asyncDataGuard.unlock();
        for( const auto& i: tmpData ) {
            i->process( this );
        }
    }
}

void JsVlcPlayer::setupBuffer( const RV32FrameSetupData& frameData )
{
    using namespace v8;

    std::shared_ptr<RV32VideoFrame> videoFrame = frameData.videoFrame.lock();

    if( !videoFrame || 0 == frameData.width || 0 == frameData.height )
        return;

    assert( frameData.size );

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Object> global = isolate->GetCurrentContext()->Global();

    Local<Value> abv =
        global->Get(
            String::NewFromUtf8( isolate,
                                 "Uint8Array",
                                 v8::String::kInternalizedString ) );
    Local<Value> argv[] =
        { Integer::NewFromUnsigned( isolate, frameData.size ) };
    Local<Object> jsArray =
        Handle<Function>::Cast( abv )->NewInstance( 1, argv );

    Local<Integer> jsWidth = Integer::New( isolate, frameData.width );
    Local<Integer> jsHeight = Integer::New( isolate, frameData.height );
    Local<Integer> jsPixelFormat = Integer::New( isolate, static_cast<int>( PixelFormat::RV32 ) );

    jsArray->Set( String::NewFromUtf8( isolate, "width" ), jsWidth );
    jsArray->Set( String::NewFromUtf8( isolate, "height" ), jsHeight );
    jsArray->Set( String::NewFromUtf8( isolate, "pixelFormat" ), jsPixelFormat );

    _jsFrameBuffer.Reset( isolate, jsArray );

    videoFrame->setFrameBuffer(
        static_cast<char*>( jsArray->GetIndexedPropertiesExternalArrayData() ) );

    callCallback( CB_FrameSetup, { jsWidth, jsHeight, jsPixelFormat } );
}

void JsVlcPlayer::setupBuffer( const I420FrameSetupData& frameData )
{
    using namespace v8;

    std::shared_ptr<I420VideoFrame> videoFrame = frameData.videoFrame.lock();

    if( !videoFrame || 0 == frameData.width || 0 == frameData.height )
        return;

    assert( frameData.uPlaneOffset && frameData.vPlaneOffset && frameData.size );

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Object> global = isolate->GetCurrentContext()->Global();

    Local<Value> abv =
        global->Get(
            String::NewFromUtf8( isolate,
                                 "Uint8Array",
                                 v8::String::kInternalizedString ) );
    Local<Value> argv[] =
        { Integer::NewFromUnsigned( isolate, frameData.size ) };
    Local<Object> jsArray =
        Handle<Function>::Cast( abv )->NewInstance( 1, argv );

    Local<Integer> jsWidth = Integer::New( isolate, frameData.width );
    Local<Integer> jsHeight = Integer::New( isolate, frameData.height );
    Local<Integer> jsPixelFormat = Integer::New( isolate, static_cast<int>( PixelFormat::I420 ) );

    jsArray->Set( String::NewFromUtf8( isolate, "width" ), jsWidth );
    jsArray->Set( String::NewFromUtf8( isolate, "height" ), jsHeight );
    jsArray->Set( String::NewFromUtf8( isolate, "pixelFormat" ), jsPixelFormat );
    jsArray->Set( String::NewFromUtf8( isolate, "uOffset" ),
                  Integer::New( isolate, frameData.uPlaneOffset ) );
    jsArray->Set( String::NewFromUtf8( isolate, "vOffset" ),
                  Integer::New( isolate, frameData.vPlaneOffset ) );

    _jsFrameBuffer.Reset( isolate, jsArray );

    videoFrame->setFrameBuffer(
        static_cast<char*>( jsArray->GetIndexedPropertiesExternalArrayData() ) );

    callCallback( CB_FrameSetup, { jsWidth, jsHeight, jsPixelFormat } );
}

void JsVlcPlayer::frameUpdated()
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    callCallback( CB_FrameReady, { Local<Value>::New( Isolate::GetCurrent(), _jsFrameBuffer ) } );
}

#define SET_CALLBACK_PROPERTY( objTemplate, name, callback )                      \
    objTemplate->SetAccessor( String::NewFromUtf8( Isolate::GetCurrent(), name ), \
        [] ( v8::Local<v8::String> property,                                      \
             const v8::PropertyCallbackInfo<v8::Value>& info )                    \
        {                                                                         \
            JsVlcPlayer::getJsCallback( property, info, callback );               \
        },                                                                        \
        [] ( v8::Local<v8::String> property,                                      \
             v8::Local<v8::Value> value,                                          \
             const v8::PropertyCallbackInfo<void>& info )                         \
        {                                                                         \
            JsVlcPlayer::setJsCallback( property, value, info, callback );        \
        } )

void JsVlcPlayer::callCallback( Callbacks_e callback,
                                std::initializer_list<v8::Local<v8::Value> > list )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    if( _jsCallbacks[callback].IsEmpty() )
        return;

    std::vector<v8::Local<v8::Value> > argList = list;

    Local<Function> callbackFunc =
        Local<Function>::New( isolate, _jsCallbacks[callback] );

    callbackFunc->Call( isolate->GetCurrentContext()->Global(),
                        argList.size(), argList.data() );
}

void JsVlcPlayer::initJsApi( const v8::Handle<v8::Object>& exports )
{
    node::AtExit( [] ( void* ) { JsVlcPlayer::closeAll(); } );

    JsVlcPlaylist::initJsApi();

    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<FunctionTemplate> ct = FunctionTemplate::New( isolate, jsCreate );
    ct->SetClassName( String::NewFromUtf8( isolate, "VlcPlayer" ) );

    Local<ObjectTemplate> vlcPlayerTemplate = ct->InstanceTemplate();
    vlcPlayerTemplate->SetInternalFieldCount( 1 );

    vlcPlayerTemplate->Set( String::NewFromUtf8( isolate, "RV32" ),
                            Integer::New( isolate, static_cast<int>( PixelFormat::RV32 ) ),
                            ReadOnly );
    vlcPlayerTemplate->Set( String::NewFromUtf8( isolate, "I420" ),
                            Integer::New( isolate, static_cast<int>( PixelFormat::I420 ) ),
                            ReadOnly );

    vlcPlayerTemplate->Set( String::NewFromUtf8( isolate, "NothingSpecial" ),
                            Integer::New( isolate, libvlc_NothingSpecial ), ReadOnly );
    vlcPlayerTemplate->Set( String::NewFromUtf8( isolate, "Opening" ),
                            Integer::New( isolate, libvlc_Opening ), ReadOnly );
    vlcPlayerTemplate->Set( String::NewFromUtf8( isolate, "Buffering" ),
                            Integer::New( isolate, libvlc_Buffering ), ReadOnly );
    vlcPlayerTemplate->Set( String::NewFromUtf8( isolate, "Playing" ),
                            Integer::New( isolate, libvlc_Playing ), ReadOnly );
    vlcPlayerTemplate->Set( String::NewFromUtf8( isolate, "Paused" ),
                            Integer::New( isolate, libvlc_Paused ), ReadOnly );
    vlcPlayerTemplate->Set( String::NewFromUtf8( isolate, "Stopped" ),
                            Integer::New( isolate, libvlc_Stopped ), ReadOnly );
    vlcPlayerTemplate->Set( String::NewFromUtf8( isolate, "Ended" ),
                            Integer::New( isolate, libvlc_Ended ), ReadOnly );
    vlcPlayerTemplate->Set( String::NewFromUtf8( isolate, "Error" ),
                            Integer::New( isolate, libvlc_Error ), ReadOnly );

    vlcPlayerTemplate->SetAccessor( String::NewFromUtf8( isolate, "pixelFormat" ),
                                    jsPixelFormat, jsSetPixelFormat );

    vlcPlayerTemplate->SetAccessor( String::NewFromUtf8( isolate, "playing" ),
                                    jsPlaying );
    vlcPlayerTemplate->SetAccessor( String::NewFromUtf8( isolate, "length" ),
                                    jsLength );
    vlcPlayerTemplate->SetAccessor( String::NewFromUtf8( isolate, "state" ),
                                    jsState );
    vlcPlayerTemplate->SetAccessor( String::NewFromUtf8( isolate, "playlist" ),
                                    jsPlaylist );

    vlcPlayerTemplate->SetAccessor( String::NewFromUtf8( isolate, "position" ),
                                    jsPosition, jsSetPosition );
    vlcPlayerTemplate->SetAccessor( String::NewFromUtf8( isolate, "time" ),
                                    jsTime, jsSetTime );
    vlcPlayerTemplate->SetAccessor( String::NewFromUtf8( isolate, "volume" ),
                                    jsVolume, jsSetVolume );
    vlcPlayerTemplate->SetAccessor( String::NewFromUtf8( isolate, "mute" ),
                                    jsMute, jsSetMute );

    SET_CALLBACK_PROPERTY( vlcPlayerTemplate, "onFrameSetup", CB_FrameSetup );
    SET_CALLBACK_PROPERTY( vlcPlayerTemplate, "onFrameReady", CB_FrameReady );
    SET_CALLBACK_PROPERTY( vlcPlayerTemplate, "onFrameCleanup", CB_FrameCleanup );

    SET_CALLBACK_PROPERTY( vlcPlayerTemplate, "onMediaChanged", CB_MediaPlayerMediaChanged );
    SET_CALLBACK_PROPERTY( vlcPlayerTemplate, "onNothingSpecial", CB_MediaPlayerNothingSpecial );
    SET_CALLBACK_PROPERTY( vlcPlayerTemplate, "onOpening", CB_MediaPlayerOpening );
    SET_CALLBACK_PROPERTY( vlcPlayerTemplate, "onBuffering", CB_MediaPlayerBuffering );
    SET_CALLBACK_PROPERTY( vlcPlayerTemplate, "onPlaying", CB_MediaPlayerPlaying );
    SET_CALLBACK_PROPERTY( vlcPlayerTemplate, "onPaused", CB_MediaPlayerPaused );
    SET_CALLBACK_PROPERTY( vlcPlayerTemplate, "onForward", CB_MediaPlayerForward );
    SET_CALLBACK_PROPERTY( vlcPlayerTemplate, "onBackward", CB_MediaPlayerBackward );
    SET_CALLBACK_PROPERTY( vlcPlayerTemplate, "onEncounteredError", CB_MediaPlayerEncounteredError );
    SET_CALLBACK_PROPERTY( vlcPlayerTemplate, "onEndReached", CB_MediaPlayerEndReached );
    SET_CALLBACK_PROPERTY( vlcPlayerTemplate, "onStopped", CB_MediaPlayerStopped );

    SET_CALLBACK_PROPERTY( vlcPlayerTemplate, "onTimeChanged", CB_MediaPlayerTimeChanged );
    SET_CALLBACK_PROPERTY( vlcPlayerTemplate, "onPositionChanged", CB_MediaPlayerPositionChanged );
    SET_CALLBACK_PROPERTY( vlcPlayerTemplate, "onSeekableChanged", CB_MediaPlayerSeekableChanged );
    SET_CALLBACK_PROPERTY( vlcPlayerTemplate, "onPausableChanged", CB_MediaPlayerPausableChanged );
    SET_CALLBACK_PROPERTY( vlcPlayerTemplate, "onLengthChanged", CB_MediaPlayerLengthChanged );

    NODE_SET_PROTOTYPE_METHOD( ct, "play", jsPlay );
    NODE_SET_PROTOTYPE_METHOD( ct, "pause", jsPause );
    NODE_SET_PROTOTYPE_METHOD( ct, "togglePause", jsTogglePause );
    NODE_SET_PROTOTYPE_METHOD( ct, "stop", jsStop );
    NODE_SET_PROTOTYPE_METHOD( ct, "toggleMute", jsToggleMute );

    _jsConstructor.Reset( isolate, ct->GetFunction() );
    exports->Set( String::NewFromUtf8( isolate, "VlcPlayer" ), ct->GetFunction() );
    exports->Set( String::NewFromUtf8( isolate, "createPlayer" ), ct->GetFunction() );
}

void JsVlcPlayer::jsCreate( const v8::FunctionCallbackInfo<v8::Value>& args )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Object> thisObject = args.Holder();
    if( args.IsConstructCall() && thisObject->InternalFieldCount() > 0 ) {
        Local<Array> options;
        if( args.Length() == 1 && args[0]->IsArray() ) {
            options = Local<Array>::Cast( args[0] );
        }

        JsVlcPlayer* jsPlayer = new JsVlcPlayer( options );
        jsPlayer->Wrap( thisObject );
        args.GetReturnValue().Set( thisObject );
    } else {
        Local<Value> argv[] = { args[0] };
        Local<Function> constructor =
            Local<Function>::New( isolate, _jsConstructor );
        args.GetReturnValue().Set( constructor->NewInstance( sizeof( argv ) / sizeof( argv[0] ), argv ) );
    }
}

void JsVlcPlayer::jsPixelFormat( v8::Local<v8::String> property,
                                 const v8::PropertyCallbackInfo<v8::Value>& info )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( info.Holder() );

    info.GetReturnValue().Set(
        Integer::New( isolate,
                      static_cast<int>( jsPlayer->_pixelFormat ) ) );
}

void JsVlcPlayer::jsSetPixelFormat( v8::Local<v8::String> property,
                                    v8::Local<v8::Value> value,
                                    const v8::PropertyCallbackInfo<void>& info )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( info.Holder() );

    Local<Integer> jsPixelFormat = Local<Integer>::Cast( value );
    if( !jsPixelFormat.IsEmpty() ) {
        switch( jsPixelFormat->Value() ) {
            case static_cast<decltype( jsPixelFormat->Value() )>( PixelFormat::RV32 ):
                jsPlayer->_pixelFormat = PixelFormat::RV32;
                break;
            case static_cast<decltype( jsPixelFormat->Value() )>( PixelFormat::I420 ):
                jsPlayer->_pixelFormat = PixelFormat::I420;
                break;
        }
    }
}

void JsVlcPlayer::jsPlaying( v8::Local<v8::String> property,
                             const v8::PropertyCallbackInfo<v8::Value>& info )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( info.Holder() );
    vlc::player& player = jsPlayer->_player;

    info.GetReturnValue().Set( Boolean::New( isolate, player.is_playing() ) );
}

void JsVlcPlayer::jsLength( v8::Local<v8::String> property,
                            const v8::PropertyCallbackInfo<v8::Value>& info )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( info.Holder() );
    vlc::player& player = jsPlayer->_player;

    info.GetReturnValue().Set(
        Number::New( isolate, static_cast<double>( player.get_length() ) ) );
}

void JsVlcPlayer::jsState( v8::Local<v8::String> property,
                           const v8::PropertyCallbackInfo<v8::Value>& info )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( info.Holder() );
    vlc::player& player = jsPlayer->_player;

    info.GetReturnValue().Set( Integer::New( isolate, player.get_state() ) );
}

void JsVlcPlayer::jsPlaylist( v8::Local<v8::String> property,
                           const v8::PropertyCallbackInfo<v8::Value>& info )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( info.Holder() );

    info.GetReturnValue().Set( Local<Object>::New( isolate, jsPlayer->_jsPlaylist ) );
}

void JsVlcPlayer::jsPosition( v8::Local<v8::String> property,
                              const v8::PropertyCallbackInfo<v8::Value>& info )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( info.Holder() );
    vlc::player& player = jsPlayer->_player;

    info.GetReturnValue().Set( Number::New( isolate, player.get_position() ) );
}

void JsVlcPlayer::jsSetPosition( v8::Local<v8::String> property,
                                 v8::Local<v8::Value> value,
                                 const v8::PropertyCallbackInfo<void>& info )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( info.Holder() );
    vlc::player& player = jsPlayer->_player;

    Local<Number> jsPosition = Local<Number>::Cast( value );
    if( !jsPosition.IsEmpty() )
        player.set_position( static_cast<float>( jsPosition->Value() ) );
}

void JsVlcPlayer::jsTime( v8::Local<v8::String> property,
                          const v8::PropertyCallbackInfo<v8::Value>& info )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( info.Holder() );
    vlc::player& player = jsPlayer->_player;

    info.GetReturnValue().Set(
        Number::New( isolate, static_cast<double>( player.get_time() ) ) );
}

void JsVlcPlayer::jsSetTime( v8::Local<v8::String> property,
                             v8::Local<v8::Value> value,
                             const v8::PropertyCallbackInfo<void>& info )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( info.Holder() );
    vlc::player& player = jsPlayer->_player;

    Local<Number> jsTime = Local<Number>::Cast( value );
    if( !jsTime.IsEmpty() )
        player.set_time( static_cast<libvlc_time_t>( jsTime->Value() ) );
}

void JsVlcPlayer::jsVolume( v8::Local<v8::String> property,
                            const v8::PropertyCallbackInfo<v8::Value>& info )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( info.Holder() );
    vlc::player& player = jsPlayer->_player;

    info.GetReturnValue().Set( Number::New( isolate, player.audio().get_volume() ) );
}

void JsVlcPlayer::jsSetVolume( v8::Local<v8::String> property,
                               v8::Local<v8::Value> value,
                               const v8::PropertyCallbackInfo<void>& info )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( info.Holder() );
    vlc::player& player = jsPlayer->_player;

    Local<Integer> jsVolume = Local<Integer>::Cast( value );
    if( !jsVolume.IsEmpty() && jsVolume->Value() > 0 )
        player.audio().set_volume( static_cast<unsigned>( jsVolume->Value() ) );
}

void JsVlcPlayer::jsMute( v8::Local<v8::String> property,
                          const v8::PropertyCallbackInfo<v8::Value>& info )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( info.Holder() );
    vlc::player& player = jsPlayer->_player;

    info.GetReturnValue().Set( Boolean::New( isolate, player.audio().is_muted() ) );
}

void JsVlcPlayer::jsSetMute( v8::Local<v8::String> property,
                             v8::Local<v8::Value> value,
                             const v8::PropertyCallbackInfo<void>& info )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( info.Holder() );
    vlc::player& player = jsPlayer->_player;

    if( !value.IsEmpty() )
        player.audio().set_mute( value->IsTrue() );
}

void JsVlcPlayer::jsPlay( const v8::FunctionCallbackInfo<v8::Value>& args )
{
    using namespace v8;

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( args.Holder() );
    vlc::player& player = jsPlayer->_player;

    if( args.Length() == 0 ) {
        player.play();
    } else if( args.Length() ==  1 ) {
        String::Utf8Value mrl( args[0]->ToString() );
        if( mrl.length() ) {
            player.clear_items();
            const int idx = player.add_media( *mrl );
            if( idx >= 0 ) {
                player.play( idx );
            }
        }
    }
}

void JsVlcPlayer::jsPause( const v8::FunctionCallbackInfo<v8::Value>& args )
{
    if( args.Length() != 0 )
        return;

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( args.Holder() );
    vlc::player& player = jsPlayer->_player;

    player.pause();
}

void JsVlcPlayer::jsTogglePause( const v8::FunctionCallbackInfo<v8::Value>& args )
{
    if( args.Length() != 0 )
        return;

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( args.Holder() );
    vlc::player& player = jsPlayer->_player;

    player.togglePause();
}

void JsVlcPlayer::jsStop( const v8::FunctionCallbackInfo<v8::Value>& args )
{
    if( args.Length() != 0 )
        return;

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( args.Holder() );
    vlc::player& player = jsPlayer->_player;

    player.stop();
}

void JsVlcPlayer::jsToggleMute( const v8::FunctionCallbackInfo<v8::Value>& args )
{
    if( args.Length() != 0 )
        return;

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( args.Holder() );
    vlc::player& player = jsPlayer->_player;

    player.audio().toggle_mute();
}

void JsVlcPlayer::getJsCallback( v8::Local<v8::String> property,
                                 const v8::PropertyCallbackInfo<v8::Value>& info,
                                 Callbacks_e callback )
{
    using namespace v8;

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( info.Holder() );

    if( jsPlayer->_jsCallbacks[callback].IsEmpty() )
        return;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Function> callbackFunc =
        Local<Function>::New( isolate, jsPlayer->_jsCallbacks[callback] );

    info.GetReturnValue().Set( callbackFunc );
}

void JsVlcPlayer::setJsCallback( v8::Local<v8::String> property,
                                 v8::Local<v8::Value> value,
                                 const v8::PropertyCallbackInfo<void>& info,
                                 Callbacks_e callback )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( info.Holder() );

    Local<Function> callbackFunc = Local<Function>::Cast( value );
    if( !callbackFunc.IsEmpty() )
        jsPlayer->_jsCallbacks[callback].Reset( isolate, callbackFunc );
}

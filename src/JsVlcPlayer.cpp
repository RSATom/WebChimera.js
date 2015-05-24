#include "JsVlcPlayer.h"

v8::Persistent<v8::Function> JsVlcPlayer::_jsConstructor;

///////////////////////////////////////////////////////////////////////////////
struct JsVlcPlayer::AsyncData
{
    virtual void process( JsVlcPlayer* ) = 0;
};

///////////////////////////////////////////////////////////////////////////////
struct JsVlcPlayer::FrameSetupData : public JsVlcPlayer::AsyncData
{
    FrameSetupData( unsigned width, unsigned height, const std::string& pixelFormat ) :
        width( width ), height( height ), pixelFormat( pixelFormat ) {}

    void process( JsVlcPlayer* ) override;

    const unsigned width;
    const unsigned height;
    const std::string pixelFormat;
};

void JsVlcPlayer::FrameSetupData::process( JsVlcPlayer* jsPlayer )
{
    jsPlayer->setupBuffer( width, height, pixelFormat );
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
JsVlcPlayer::JsVlcPlayer() :
    _libvlc( nullptr ), _jsRawFrameBuffer( nullptr )
{
    _libvlc = libvlc_new( 0, nullptr );
    assert( _libvlc );
    if( _player.open( _libvlc ) ) {
        vlc::basic_vmem_wrapper::open( &_player.basic_player() );
    } else {
        assert( false );
    }

    uv_loop_t* loop = uv_default_loop();

    uv_async_init( loop, &_async,
        [] ( uv_async_t* handle ) {
            if( handle->data )
                reinterpret_cast<JsVlcPlayer*>( handle->data )->handleAsync();
        }
    );
    _async.data = this;
}

JsVlcPlayer::~JsVlcPlayer()
{
    vlc::basic_vmem_wrapper::close();

    _async.data = nullptr;
    uv_close( reinterpret_cast<uv_handle_t*>( &_async ), 0 );
}

unsigned JsVlcPlayer::video_format_cb( char* chroma,
                                       unsigned* width, unsigned* height,
                                       unsigned* pitches, unsigned* lines )
{
    memcpy( chroma, vlc::DEF_CHROMA, sizeof( vlc::DEF_CHROMA ) - 1 );
    *pitches = *width * vlc::DEF_PIXEL_BYTES;
    *lines = *height;

    _tmpFrameBuffer.resize( *pitches * *lines );

    _asyncData.push_back( std::make_shared<FrameSetupData>( *width, *height, vlc::DEF_CHROMA ) );
    uv_async_send( &_async );

    return 1;
}

void JsVlcPlayer::video_cleanup_cb()
{
    if( !_tmpFrameBuffer.empty() )
        _tmpFrameBuffer.swap( std::vector<char>() );

    _asyncData.push_back( std::make_shared<CallbackData>( CB_FRAME_CLEANUP ) );
    uv_async_send( &_async );
}

void* JsVlcPlayer::video_lock_cb( void** planes )
{
    if( _tmpFrameBuffer.empty() ) {
        *planes = _jsRawFrameBuffer;
    } else {
        if( _jsRawFrameBuffer ) {
            _tmpFrameBuffer.swap( std::vector<char>() );
            *planes = _jsRawFrameBuffer;
        } else {
            *planes = _tmpFrameBuffer.data();
        }
    }

    return nullptr;
}

void JsVlcPlayer::video_unlock_cb( void* /*picture*/, void *const * /*planes*/ )
{
}

void JsVlcPlayer::video_display_cb( void* /*picture*/ )
{
    _asyncData.push_back( std::make_shared<FrameUpdated>() );
    uv_async_send( &_async );
}

void JsVlcPlayer::handleAsync()
{
    while( !_asyncData.empty() ) {
        std::deque<std::shared_ptr<AsyncData> > tmpData;
        _asyncData.swap( tmpData );
        for( const auto& i: tmpData ) {
            i->process( this );
        }
    }
}

void JsVlcPlayer::setupBuffer( unsigned width, unsigned height, const std::string& pixelFormat )
{
    using namespace v8;

    if( 0 == width || 0 == height )
        return;

    const unsigned frameBufferSize = width * height * vlc::DEF_PIXEL_BYTES;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Object> global = isolate->GetCurrentContext()->Global();

    Local<Value> abv =
        global->Get(
            String::NewFromUtf8( isolate,
                                 "Uint8Array",
                                 v8::String::kInternalizedString ) );
    Local<Value> argv[] =
        { Integer::NewFromUnsigned( isolate, frameBufferSize ) };
    Local<Object> jsArray =
        Handle<Function>::Cast( abv )->NewInstance( 1, argv );

    Local<Integer> jsWidth = Integer::New( isolate, width );
    Local<Integer> jsHeight = Integer::New( isolate, height );
    Local<String> jsPixelFormat = String::NewFromUtf8( isolate, pixelFormat.c_str() );

    jsArray->Set( String::NewFromUtf8( isolate, "width" ), jsWidth );
    jsArray->Set( String::NewFromUtf8( isolate, "height" ), jsHeight );
    jsArray->Set( String::NewFromUtf8( isolate, "pixelFormat" ), jsPixelFormat );

    _jsFrameBuffer.Reset( isolate, jsArray );

    _jsRawFrameBuffer =
        static_cast<char*>( jsArray->GetIndexedPropertiesExternalArrayData() );

    callCallback( CB_FRAME_SETUP, { jsWidth, jsHeight, jsPixelFormat } );
}

void JsVlcPlayer::frameUpdated()
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    callCallback( CB_FRAME_READY, { Local<Value>::New( Isolate::GetCurrent(), _jsFrameBuffer ) } );
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

void JsVlcPlayer::initJsApi()
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();

    Local<FunctionTemplate> ct = FunctionTemplate::New( isolate, jsCreate );
    ct->SetClassName( String::NewFromUtf8( isolate, "VlcPlayer" ) );

    Local<ObjectTemplate> vlcPlayerTemplate = ct->InstanceTemplate();
    vlcPlayerTemplate->SetInternalFieldCount( 1 );

    SET_CALLBACK_PROPERTY( vlcPlayerTemplate, "onFrameSetup", CB_FRAME_SETUP );
    SET_CALLBACK_PROPERTY( vlcPlayerTemplate, "onFrameReady", CB_FRAME_READY );
    SET_CALLBACK_PROPERTY( vlcPlayerTemplate, "onFrameCleanup", CB_FRAME_CLEANUP );

    NODE_SET_PROTOTYPE_METHOD( ct, "play", jsPlay );
    NODE_SET_PROTOTYPE_METHOD( ct, "stop", jsStop );

    _jsConstructor.Reset( isolate, ct->GetFunction() );
}

void JsVlcPlayer::jsCreate( const v8::FunctionCallbackInfo<v8::Value>& args )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    if( args.IsConstructCall() ) {
        JsVlcPlayer* jsPlayer = new JsVlcPlayer;
        jsPlayer->Wrap( args.This() );
        args.GetReturnValue().Set( args.This() );
    } else {
        Local<Function> constructor =
            Local<Function>::New( isolate, _jsConstructor );
        args.GetReturnValue().Set( constructor->NewInstance( 0, nullptr ) );
    }
}

void JsVlcPlayer::jsPlay( const v8::FunctionCallbackInfo<v8::Value>& args )
{
    using namespace v8;

    if( args.Length() != 1 )
        return;

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( args.Holder() );
    vlc::player& player = jsPlayer->_player;

    String::Utf8Value mrl( args[0]->ToString() );
    if( mrl.length() ) {
        player.clear_items();
        const int idx = player.add_media( *mrl );
        if( idx >= 0 ) {
            player.play( idx );
        }
    }
}

void JsVlcPlayer::jsStop( const v8::FunctionCallbackInfo<v8::Value>& args )
{
    if( args.Length() != 0 )
        return;

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( args.Holder() );
    vlc::player& player = jsPlayer->_player;

    player.stop();
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

#include "JsVlcPlayer.h"

v8::Persistent<v8::Function> JsVlcPlayer::_jsConstructor;

JsVlcPlayer::JsVlcPlayer( const v8::Local<v8::Function>& renderCallback ) :
    _libvlc( nullptr ), _frameWidth( 0 ), _frameHeight( 0 ),
    _jsFrameBufferSize( 0 ), _jsRawFrameBuffer( nullptr )
{
    _jsRenderCallback.Reset( v8::Isolate::GetCurrent(), renderCallback );

    _libvlc = libvlc_new( 0, nullptr );
    assert( _libvlc );
    if( _player.open( _libvlc ) ) {
        vlc::vmem::open( &_player.basic_player() );
    } else {
        assert( false );
    }

    uv_loop_t* loop = uv_default_loop();

    _formatSetupAsync.data = this;
    uv_async_init( loop, &_formatSetupAsync,
        [] ( uv_async_t* handle ) {
            if( handle->data )
                reinterpret_cast<JsVlcPlayer*>( handle->data )->setupBuffer();
        }
    );

    _frameUpdatedAsync.data = this;
    uv_async_init( loop, &_frameUpdatedAsync,
        [] ( uv_async_t* handle ) {
            if( handle->data )
                reinterpret_cast<JsVlcPlayer*>( handle->data )->frameUpdated() ;
        }
    );
}

JsVlcPlayer::~JsVlcPlayer()
{
    vlc::vmem::close();

    _formatSetupAsync.data = 0;
    uv_close( reinterpret_cast<uv_handle_t*>( &_formatSetupAsync ), 0 );

    _frameUpdatedAsync.data = 0;
    uv_close( reinterpret_cast<uv_handle_t*>( &_frameUpdatedAsync ), 0 );
}

void JsVlcPlayer::on_format_setup()
{
    _frameWidth = width();
    _frameHeight = height();

    uv_async_send( &_formatSetupAsync );
}

void JsVlcPlayer::on_frame_ready( const std::vector<char>* frameBuf )
{
    if( frameBuf && !frameBuf->empty() && _jsRawFrameBuffer ) {
        assert( _jsFrameBufferSize <= frameBuf->size() );
        memcpy( _jsRawFrameBuffer, frameBuf->data(), _jsFrameBufferSize );
        uv_async_send( &_frameUpdatedAsync );
    }
}

void JsVlcPlayer::on_frame_cleanup()
{
    uv_async_send( &_frameUpdatedAsync );
}

void JsVlcPlayer::setupBuffer()
{
    using namespace v8;

    if( 0 == _frameWidth || 0 == _frameHeight )
        return;

    _jsFrameBufferSize = _frameWidth * _frameHeight * vlc::DEF_PIXEL_BYTES;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Object> global = isolate->GetCurrentContext()->Global();

    Local<Value> abv =
        global->Get(
            String::NewFromUtf8( isolate,
                                 "Uint8Array",
                                 v8::String::kInternalizedString ) );
    Local<Value> argv[] =
        { Integer::NewFromUnsigned( isolate, _jsFrameBufferSize ) };
    Local<Object> array =
        Handle<Function>::Cast( abv )->NewInstance( 1, argv );

    array->Set( String::NewFromUtf8( isolate, "width" ),
                Integer::New( isolate, _frameWidth ) );
    array->Set( String::NewFromUtf8( isolate, "height" ),
                Integer::New( isolate, _frameHeight) );

    _jsFrameBuffer.Reset( isolate, array );

    _jsRawFrameBuffer =
        static_cast<char*>( array->GetIndexedPropertiesExternalArrayData() );
}

void JsVlcPlayer::frameUpdated()
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Function> renderCallback =
        Local<Function>::New( isolate, _jsRenderCallback );

    Local<Value> argv[] =
        { Local<Object>::New( isolate, _jsFrameBuffer ) };

    renderCallback->Call( isolate->GetCurrentContext()->Global(),
                          sizeof( argv ) / sizeof( argv[0] ), argv );
}

void JsVlcPlayer::initJsApi()
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();

    Local<FunctionTemplate> ct = FunctionTemplate::New( isolate, jsCreate );
    ct->SetClassName( String::NewFromUtf8( isolate, "VlcPlayer" ) );
    ct->InstanceTemplate()->SetInternalFieldCount( 1 );

    NODE_SET_PROTOTYPE_METHOD( ct, "getVideoFrame", jsGetVideoFrame );
    NODE_SET_PROTOTYPE_METHOD( ct, "play", jsPlay );
    NODE_SET_PROTOTYPE_METHOD( ct, "stop", jsStop );

    _jsConstructor.Reset( isolate, ct->GetFunction() );
}

void JsVlcPlayer::jsCreate( const v8::FunctionCallbackInfo<v8::Value>& args )
{
    using namespace v8;

    if( args.Length() != 1 )
        return;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Function> renderCallback = Local<Function>::Cast( args[0] );

    if( args.IsConstructCall() ) {
        JsVlcPlayer* jsPlayer = new JsVlcPlayer( renderCallback );
        jsPlayer->Wrap( args.This() );
        args.GetReturnValue().Set( args.This() );
    } else {
        Local<Value> argv[] = { renderCallback };
        Local<Function> constructor =
            Local<Function>::New( isolate, _jsConstructor );
        args.GetReturnValue().Set(
            constructor->NewInstance( sizeof( argv ) / sizeof( argv[0] ),
                                      argv ) );
    }
}

void JsVlcPlayer::jsGetVideoFrame( const v8::FunctionCallbackInfo<v8::Value>& args )
{
    using namespace v8;

    if( args.Length() != 0 )
        return;

    JsVlcPlayer* jsPlayer = ObjectWrap::Unwrap<JsVlcPlayer>( args.Holder() );

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    args.GetReturnValue().Set(
        v8::Local<Object>::New( isolate, jsPlayer->_jsFrameBuffer ) );
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

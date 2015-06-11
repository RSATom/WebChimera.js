#include "JsVlcPlaylist.h"

#include "JsVlcPlayer.h"

v8::Persistent<v8::Function> JsVlcPlaylist::_jsConstructor;

JsVlcPlaylist::JsVlcPlaylist( v8::Local<v8::Object>& thisObject, JsVlcPlayer* jsPlayer ) :
    _jsPlayer( jsPlayer )
{
    Wrap( thisObject );
}

v8::UniquePersistent<v8::Object> JsVlcPlaylist::create( JsVlcPlayer& player )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Function> constructor =
        Local<Function>::New( isolate, _jsConstructor );

    Local<Value> argv[] = { player.handle() };

    return { isolate, constructor->NewInstance( sizeof( argv ) / sizeof( argv[0] ), argv ) };
}

void JsVlcPlaylist::initJsApi()
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<FunctionTemplate> ct = FunctionTemplate::New( isolate, jsCreate );
    ct->SetClassName( String::NewFromUtf8( isolate, "VlcPlaylist" ) );

    Local<ObjectTemplate> jsTemplate = ct->InstanceTemplate();
    jsTemplate->SetInternalFieldCount( 1 );

    jsTemplate->Set( String::NewFromUtf8( isolate, "Normal" ),
                     Integer::New( isolate, static_cast<int>( PlaybackMode::Normal ) ),
                     ReadOnly );
    jsTemplate->Set( String::NewFromUtf8( isolate, "Loop" ),
                     Integer::New( isolate, static_cast<int>( PlaybackMode::Loop ) ),
                     ReadOnly );
    jsTemplate->Set( String::NewFromUtf8( isolate, "Single" ),
                     Integer::New( isolate, static_cast<int>( PlaybackMode::Single ) ),
                     ReadOnly );

    _jsConstructor.Reset( isolate, ct->GetFunction() );
}

void JsVlcPlaylist::jsCreate( const v8::FunctionCallbackInfo<v8::Value>& args )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Object> thisObject = args.Holder();
    if( args.IsConstructCall() && thisObject->InternalFieldCount() > 0 ) {
        JsVlcPlayer* jsPlayer =
            ObjectWrap::Unwrap<JsVlcPlayer>( Handle<Object>::Cast( args[0] ) );
        if( jsPlayer ) {
            JsVlcPlaylist* jsPlaylist = new JsVlcPlaylist( thisObject, jsPlayer );
            args.GetReturnValue().Set( thisObject );
        }
    } else {
        Local<Function> constructor =
            Local<Function>::New( isolate, _jsConstructor );
        Local<Value> argv[] = { args[0] };
        args.GetReturnValue().Set(
            constructor->NewInstance( sizeof( argv ) / sizeof( argv[0] ), argv ) );
    }
}

void JsVlcPlaylist::jsMode( v8::Local<v8::String> property,
                            const v8::PropertyCallbackInfo<v8::Value>& info )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    JsVlcPlaylist* playlist = ObjectWrap::Unwrap<JsVlcPlaylist>( info.Holder() );

    info.GetReturnValue().Set(
        Integer::New( isolate,
                      static_cast<int>( playlist->_jsPlayer->player().get_playback_mode() ) ) );
}

void JsVlcPlaylist::jsSetMode( v8::Local<v8::String> property,
                               v8::Local<v8::Value> value,
                               const v8::PropertyCallbackInfo<void>& info )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    JsVlcPlaylist* jsPlaylist = ObjectWrap::Unwrap<JsVlcPlaylist>( info.Holder() );

    vlc::player& player = jsPlaylist->_jsPlayer->player();

    Local<Integer> jsMode = Local<Integer>::Cast( value );
    if( !jsMode.IsEmpty() ) {
        switch( jsMode->Value() ) {
            case static_cast<decltype( jsMode->Value() )>( PlaybackMode::Normal ):
                player.set_playback_mode( vlc::mode_normal );
                break;
            case static_cast<decltype( jsMode->Value() )>( PlaybackMode::Loop ):
                player.set_playback_mode( vlc::mode_loop );
                break;
            case static_cast<decltype( jsMode->Value() )>( PlaybackMode::Single ):
                player.set_playback_mode( vlc::mode_single );
                break;
        }
    }
}

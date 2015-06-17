#include "JsPlaylistItems.h"

#include "NodeTools.h"
#include "JsVlcPlayer.h"

v8::Persistent<v8::Function> JsVlcPlaylistItems::_jsConstructor;

JsVlcPlaylistItems::JsVlcPlaylistItems( v8::Local<v8::Object>& thisObject, JsVlcPlayer* jsPlayer ) :
    _jsPlayer( jsPlayer )
{
    Wrap( thisObject );
}

v8::UniquePersistent<v8::Object> JsVlcPlaylistItems::create( JsVlcPlayer& player )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Function> constructor =
        Local<Function>::New( isolate, _jsConstructor );

    Local<Value> argv[] = { player.handle() };

    return { isolate, constructor->NewInstance( sizeof( argv ) / sizeof( argv[0] ), argv ) };
}


void JsVlcPlaylistItems::initJsApi()
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<FunctionTemplate> ct = FunctionTemplate::New( isolate, jsCreate );
    ct->SetClassName( String::NewFromUtf8( isolate, "JsVlcPlaylistItems" ) );

    Local<ObjectTemplate> jsTemplate = ct->InstanceTemplate();
    jsTemplate->SetInternalFieldCount( 1 );

    SET_RO_PROPERTY( jsTemplate, "count", &JsVlcPlaylistItems::count );

    SET_METHOD( ct, "clear", &JsVlcPlaylistItems::clear );
    SET_METHOD( ct, "remove", &JsVlcPlaylistItems::remove );

    _jsConstructor.Reset( isolate, ct->GetFunction() );
}

void JsVlcPlaylistItems::jsCreate( const v8::FunctionCallbackInfo<v8::Value>& args )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Object> thisObject = args.Holder();
    if( args.IsConstructCall() && thisObject->InternalFieldCount() > 0 ) {
        JsVlcPlayer* jsPlayer =
            ObjectWrap::Unwrap<JsVlcPlayer>( Handle<Object>::Cast( args[0] ) );
        if( jsPlayer ) {
            JsVlcPlaylistItems* jsPlaylist = new JsVlcPlaylistItems( thisObject, jsPlayer );
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


unsigned JsVlcPlaylistItems::count()
{
    return _jsPlayer->player().item_count();
}

void JsVlcPlaylistItems::clear()
{
    return _jsPlayer->player().clear_items();
}

bool JsVlcPlaylistItems::remove( unsigned int idx )
{
   return _jsPlayer->player().delete_item( idx );
}

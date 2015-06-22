#include "JsVlcPlaylistItems.h"

#include "NodeTools.h"
#include "JsVlcPlayer.h"
#include "JsVlcMedia.h"

v8::Persistent<v8::Function> JsVlcPlaylistItems::_jsConstructor;

void JsVlcPlaylistItems::initJsApi()
{
    JsVlcMedia::initJsApi();

    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<FunctionTemplate> constructorTemplate = FunctionTemplate::New( isolate, jsCreate );
    constructorTemplate->SetClassName(
        String::NewFromUtf8( isolate, "VlcPlaylistItems", v8::String::kInternalizedString ) );

    Local<ObjectTemplate> protoTemplate = constructorTemplate->PrototypeTemplate();
    Local<ObjectTemplate> instanceTemplate = constructorTemplate->InstanceTemplate();
    instanceTemplate->SetInternalFieldCount( 1 );

    SET_RO_PROPERTY( instanceTemplate, "count", &JsVlcPlaylistItems::count );

    SET_RO_INDEXED_PROPERTY( instanceTemplate, &JsVlcPlaylistItems::item );

    SET_METHOD( constructorTemplate, "clear", &JsVlcPlaylistItems::clear );
    SET_METHOD( constructorTemplate, "remove", &JsVlcPlaylistItems::remove );

    Local<Function> constructor = constructorTemplate->GetFunction();
    _jsConstructor.Reset( isolate, constructor );
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

JsVlcPlaylistItems::JsVlcPlaylistItems( v8::Local<v8::Object>& thisObject, JsVlcPlayer* jsPlayer ) :
    _jsPlayer( jsPlayer )
{
    Wrap( thisObject );
}

v8::Local<v8::Object> JsVlcPlaylistItems::item( uint32_t index )
{
    return JsVlcMedia::create( *_jsPlayer, _jsPlayer->player().get_media( index ) );
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

#include "JsVlcVideo.h"

#include "NodeTools.h"
#include "JsVlcPlayer.h"

v8::Persistent<v8::Function> JsVlcVideo::_jsConstructor;

void JsVlcVideo::initJsApi()
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<FunctionTemplate> constructorTemplate = FunctionTemplate::New( isolate, jsCreate );
    constructorTemplate->SetClassName(
        String::NewFromUtf8( isolate, "VlcVideo", v8::String::kInternalizedString ) );

    Local<ObjectTemplate> protoTemplate = constructorTemplate->PrototypeTemplate();
    Local<ObjectTemplate> instanceTemplate = constructorTemplate->InstanceTemplate();
    instanceTemplate->SetInternalFieldCount( 1 );

    SET_RO_PROPERTY( instanceTemplate, "count", &JsVlcVideo::count );

    SET_RW_PROPERTY( instanceTemplate, "track", &JsVlcVideo::track, &JsVlcVideo::setTrack );

    Local<Function> constructor = constructorTemplate->GetFunction();
    _jsConstructor.Reset( isolate, constructor );
}

v8::UniquePersistent<v8::Object> JsVlcVideo::create( JsVlcPlayer& player )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Function> constructor =
        Local<Function>::New( isolate, _jsConstructor );

    Local<Value> argv[] = { player.handle() };

    return { isolate, constructor->NewInstance( sizeof( argv ) / sizeof( argv[0] ), argv ) };
}

void JsVlcVideo::jsCreate( const v8::FunctionCallbackInfo<v8::Value>& args )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Object> thisObject = args.Holder();
    if( args.IsConstructCall() && thisObject->InternalFieldCount() > 0 ) {
        JsVlcPlayer* jsPlayer =
            ObjectWrap::Unwrap<JsVlcPlayer>( Handle<Object>::Cast( args[0] ) );
        if( jsPlayer ) {
            JsVlcVideo* jsPlaylist = new JsVlcVideo( thisObject, jsPlayer );
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

JsVlcVideo::JsVlcVideo( v8::Local<v8::Object>& thisObject, JsVlcPlayer* jsPlayer ) :
    _jsPlayer( jsPlayer )
{
    Wrap( thisObject );
}

unsigned JsVlcVideo::count()
{
    return _jsPlayer->player().video().track_count();
}

int JsVlcVideo::track()
{
    return _jsPlayer->player().video().get_track();
}

void JsVlcVideo::setTrack( unsigned track )
{
    _jsPlayer->player().video().set_track( track );
}

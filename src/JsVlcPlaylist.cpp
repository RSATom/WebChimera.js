#include "JsVlcPlaylist.h"

#include "JsVlcPlayer.h"

v8::Persistent<v8::Function> JsVlcPlaylist::_jsConstructor;

v8::UniquePersistent<v8::Object> JsVlcPlaylist::create( JsVlcPlayer& player )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Function> constructor =
        Local<Function>::New( isolate, _jsConstructor );

    return { isolate, constructor->NewInstance( 0, nullptr ) };
}

void JsVlcPlaylist::initJsApi()
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<FunctionTemplate> ct = FunctionTemplate::New( isolate, jsCreate );
    ct->SetClassName( String::NewFromUtf8( isolate, "VlcPlaylist" ) );

    Local<ObjectTemplate> vlcPlayerTemplate = ct->InstanceTemplate();
    vlcPlayerTemplate->SetInternalFieldCount( 1 );

    _jsConstructor.Reset( isolate, ct->GetFunction() );
}

void JsVlcPlaylist::jsCreate( const v8::FunctionCallbackInfo<v8::Value>& args )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Object> thisObject = args.Holder();
    if( args.IsConstructCall() && thisObject->InternalFieldCount() > 0 ) {
        JsVlcPlaylist* jsPlaylist= new JsVlcPlaylist;
        jsPlaylist->Wrap( thisObject );
        args.GetReturnValue().Set( thisObject );
    } else {
        Local<Function> constructor =
            Local<Function>::New( isolate, _jsConstructor );
        Local<Value> argv[] = { args[0] };
        args.GetReturnValue().Set(
            constructor->NewInstance( sizeof( argv ) / sizeof( argv[0] ), argv ) );
    }
}

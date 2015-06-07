#include "JsVlcPlaylist.h"

v8::UniquePersistent<v8::Function> JsVlcPlaylist::_jsConstructor;

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
        args.GetReturnValue().Set( constructor->NewInstance( 0, nullptr ) );
    }
}

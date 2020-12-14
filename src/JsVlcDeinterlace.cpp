#include "JsVlcDeinterlace.h"

#include "NodeTools.h"
#include "JsVlcPlayer.h"

v8::Persistent<v8::Function> JsVlcDeinterlace::_jsConstructor;

void JsVlcDeinterlace::initJsApi()
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    Local<Context> context = isolate->GetCurrentContext();
    HandleScope scope(isolate);

    Local<FunctionTemplate> constructorTemplate = FunctionTemplate::New(isolate, jsCreate);
    constructorTemplate->SetClassName(
        String::NewFromUtf8(isolate, "VlcDeinterlace", NewStringType::kInternalized).ToLocalChecked());

    Local<ObjectTemplate> protoTemplate = constructorTemplate->PrototypeTemplate();
    Local<ObjectTemplate> instanceTemplate = constructorTemplate->InstanceTemplate();
    instanceTemplate->SetInternalFieldCount(1);

    SET_METHOD(constructorTemplate, "enable", &JsVlcDeinterlace::enable);
    SET_METHOD(constructorTemplate, "disable", &JsVlcDeinterlace::disable);

    Local<Function> constructor = constructorTemplate->GetFunction(context).ToLocalChecked();
    _jsConstructor.Reset(isolate, constructor);
}

v8::UniquePersistent<v8::Object> JsVlcDeinterlace::create(JsVlcPlayer& player)
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    Local<Context> context = isolate->GetCurrentContext();
    HandleScope scope(isolate);

    Local<Function> constructor =
        Local<Function>::New(isolate, _jsConstructor);

    Local<Value> argv[] = { player.handle() };

    return {
        isolate,
        constructor->NewInstance(
            context,
            sizeof(argv) / sizeof(argv[0]), argv).ToLocalChecked()
    };
}

void JsVlcDeinterlace::jsCreate(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    Local<Context> context = isolate->GetCurrentContext();

    Local<Object> thisObject = args.Holder();
    if(args.IsConstructCall() && thisObject->InternalFieldCount() > 0) {
        JsVlcPlayer* jsPlayer =
            ObjectWrap::Unwrap<JsVlcPlayer>(Handle<Object>::Cast(args[0]));
        if(jsPlayer) {
            JsVlcDeinterlace* jsPlaylist = new JsVlcDeinterlace(thisObject, jsPlayer);
            args.GetReturnValue().Set(thisObject);
        }
    } else {
        Local<Function> constructor =
            Local<Function>::New(isolate, _jsConstructor);
        Local<Value> argv[] = { args[0] };
        args.GetReturnValue().Set(
            constructor->NewInstance(context, sizeof(argv) / sizeof(argv[0]), argv).ToLocalChecked());
    }
}

JsVlcDeinterlace::JsVlcDeinterlace(v8::Local<v8::Object>& thisObject, JsVlcPlayer* jsPlayer) :
    _jsPlayer(jsPlayer)
{
    Wrap(thisObject);
}

void JsVlcDeinterlace::enable(const std::string& mode)
{
    libvlc_video_set_deinterlace(_jsPlayer->player().get_mp(), mode.c_str());
}

void JsVlcDeinterlace::disable()
{
    libvlc_video_set_deinterlace(_jsPlayer->player().get_mp(), nullptr);
}

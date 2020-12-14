#include "JsVlcSubtitles.h"

#include "NodeTools.h"
#include "JsVlcPlayer.h"

v8::Persistent<v8::Function> JsVlcSubtitles::_jsConstructor;

void JsVlcSubtitles::initJsApi()
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);
    Local<Context> context = isolate->GetCurrentContext();

    Local<FunctionTemplate> constructorTemplate = FunctionTemplate::New(isolate, jsCreate);
    constructorTemplate->SetClassName(
        String::NewFromUtf8(isolate, "VlcSubtitles", NewStringType::kInternalized).ToLocalChecked());

    Local<ObjectTemplate> protoTemplate = constructorTemplate->PrototypeTemplate();
    Local<ObjectTemplate> instanceTemplate = constructorTemplate->InstanceTemplate();
    instanceTemplate->SetInternalFieldCount(1);

    SET_RO_INDEXED_PROPERTY(instanceTemplate, &JsVlcSubtitles::description);

    SET_RO_PROPERTY(instanceTemplate, "count", &JsVlcSubtitles::count);

    SET_RW_PROPERTY(instanceTemplate, "track", &JsVlcSubtitles::track, &JsVlcSubtitles::setTrack);
    SET_RW_PROPERTY(instanceTemplate, "delay", &JsVlcSubtitles::delay, &JsVlcSubtitles::setDelay);

    SET_METHOD(constructorTemplate, "load", &JsVlcSubtitles::load);

    Local<Function> constructor = constructorTemplate->GetFunction(context).ToLocalChecked();
    _jsConstructor.Reset(isolate, constructor);
}

v8::UniquePersistent<v8::Object> JsVlcSubtitles::create(JsVlcPlayer& player)
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
        constructor->NewInstance(context, sizeof(argv) / sizeof(argv[0]), argv).ToLocalChecked()
    };
}

void JsVlcSubtitles::jsCreate(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    Local<Context> context = isolate->GetCurrentContext();
    HandleScope scope(isolate);

    Local<Object> thisObject = args.Holder();
    if(args.IsConstructCall() && thisObject->InternalFieldCount() > 0) {
        JsVlcPlayer* jsPlayer =
            ObjectWrap::Unwrap<JsVlcPlayer>(Handle<Object>::Cast(args[0]));
        if(jsPlayer) {
            JsVlcSubtitles* jsPlaylist = new JsVlcSubtitles(thisObject, jsPlayer);
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

JsVlcSubtitles::JsVlcSubtitles(
    v8::Local<v8::Object>& thisObject, JsVlcPlayer* jsPlayer) :
    _jsPlayer(jsPlayer)
{
    Wrap(thisObject);
}

std::string JsVlcSubtitles::description(uint32_t index)
{
    vlc_player& p = _jsPlayer->player();

    std::string name;

    libvlc_track_description_t* rootDesc =
        libvlc_video_get_spu_description(p.get_mp());
    if(!rootDesc)
        return name;

    unsigned count = libvlc_video_get_spu_count(p.get_mp());
    if(count && index < count) {
        libvlc_track_description_t* desc = rootDesc;
        for(; index && desc; --index){
            desc = desc->p_next;
        }

        if (desc && desc->psz_name) {
            name = desc->psz_name;
        }
    }
    libvlc_track_description_list_release(rootDesc);

    return name;
}

unsigned JsVlcSubtitles::count()
{
    return _jsPlayer->player().subtitles().track_count();
}

int JsVlcSubtitles::track()
{
    return _jsPlayer->player().subtitles().get_track();
}

void JsVlcSubtitles::setTrack(int track)
{
    return _jsPlayer->player().subtitles().set_track(track);
}

int JsVlcSubtitles::delay()
{
    return static_cast<int>(_jsPlayer->player().subtitles().get_delay());
}

void JsVlcSubtitles::setDelay(int delay)
{
    _jsPlayer->player().subtitles().set_delay(delay);
}

bool JsVlcSubtitles::load(const std::string& path)
{
    return _jsPlayer->player().subtitles().load(path);
}

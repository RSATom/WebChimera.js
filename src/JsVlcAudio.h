#pragma once

#include <string>

#include <node.h>
#include <node_object_wrap.h>

class JsVlcPlayer; //#include "JsVlcPlayer.h"

class JsVlcAudio :
    public node::ObjectWrap
{
public:
    static void initJsApi();
    static v8::UniquePersistent<v8::Object> create(JsVlcPlayer& player);

    std::string description(uint32_t index);

    unsigned count();

    int track();
    void setTrack(int);

    bool muted();
    void setMuted(bool muted);

    unsigned volume();
    void setVolume(unsigned);

    int channel();
    void setChannel(unsigned);

    int delay();
    void setDelay(int);

    void toggleMute();

private:
    static void jsCreate(const v8::FunctionCallbackInfo<v8::Value>& args);
    JsVlcAudio(v8::Local<v8::Object>& thisObject, JsVlcPlayer*);

private:
    static v8::Persistent<v8::Function> _jsConstructor;

    JsVlcPlayer* _jsPlayer;
};

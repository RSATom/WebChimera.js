#pragma once

#include <string>

#include <node.h>
#include <node_object_wrap.h>

class JsVlcPlayer; //#include "JsVlcPlayer.h"

class JsVlcSubtitles :
    public node::ObjectWrap
{
public:
    static void initJsApi();
    static v8::UniquePersistent<v8::Object> create(JsVlcPlayer& player);

    std::string description(uint32_t index);

    unsigned count();

    int track();
    void setTrack(int);

    int delay();
    void setDelay(int);

   bool load(const std::string& path);

private:
    static void jsCreate(const v8::FunctionCallbackInfo<v8::Value>& args);
    JsVlcSubtitles(v8::Local<v8::Object>& thisObject, JsVlcPlayer*);

private:
    static v8::Persistent<v8::Function> _jsConstructor;

    JsVlcPlayer* _jsPlayer;
};

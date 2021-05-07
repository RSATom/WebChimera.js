#pragma once

#include <string>

#include <node.h>
#include <node_object_wrap.h>

class JsVlcPlayer; //#include "JsVlcPlayer.h"

class JsVlcDeinterlace :
    public node::ObjectWrap
{
public:
    static void initJsApi();
    static v8::UniquePersistent<v8::Object> create(JsVlcPlayer& player);

    void enable(const std::string& mode);
    void disable();

private:
    static void jsCreate(const v8::FunctionCallbackInfo<v8::Value>& args);
    JsVlcDeinterlace(v8::Local<v8::Object>& thisObject, JsVlcPlayer*);

private:
    static v8::Persistent<v8::Function> _jsConstructor;

    JsVlcPlayer* _jsPlayer;
};

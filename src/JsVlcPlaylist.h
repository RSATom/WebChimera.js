#pragma once

#include <v8.h>
#include <node_object_wrap.h>

class JsVlcPlaylist :
    public node::ObjectWrap
{
public:
    static void initJsApi();
    static void jsCreate( const v8::FunctionCallbackInfo<v8::Value>& args );

private:
    static v8::Persistent<v8::Function> _jsConstructor;
};

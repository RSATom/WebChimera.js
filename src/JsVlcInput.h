#pragma once

#include <string>

#include <v8.h>
#include <node_object_wrap.h>

class JsVlcPlayer; //#include "JsVlcPlayer.h"

class JsVlcInput :
    public node::ObjectWrap
{
public:
    static void initJsApi();
    static v8::UniquePersistent<v8::Object> create( JsVlcPlayer& player );

    double length();
    double fps();
    unsigned state();

    double position();
    void setPosition( double );

    double time();
    void setTime( double );

    double rate();
    void setRate( double );

private:
    static void jsCreate( const v8::FunctionCallbackInfo<v8::Value>& args );
    JsVlcInput( v8::Local<v8::Object>& thisObject, JsVlcPlayer* );

private:
    static v8::Persistent<v8::Function> _jsConstructor;

    JsVlcPlayer* _jsPlayer;
};

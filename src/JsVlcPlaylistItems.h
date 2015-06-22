#pragma once

#include <v8.h>
#include <node_object_wrap.h>

class JsVlcPlayer; //#include "JsVlcPlayer.h"

class JsVlcPlaylistItems :
    public node::ObjectWrap
{
public:
    static void initJsApi();
    static v8::UniquePersistent<v8::Object> create( JsVlcPlayer& player );

    v8::Local<v8::Object> item( uint32_t index );

    unsigned count();
    void clear();
    bool remove( unsigned idx );

private:
    static void jsCreate( const v8::FunctionCallbackInfo<v8::Value>& args );
    JsVlcPlaylistItems( v8::Local<v8::Object>& thisObject, JsVlcPlayer* );

private:
    static v8::Persistent<v8::Function> _jsConstructor;

    JsVlcPlayer* _jsPlayer;
};

#pragma once

#include <v8.h>
#include <node_object_wrap.h>

#include <libvlc_wrapper/vlc_player.h>

class JsVlcPlayer; //#include "JsVlcPlayer.h"

class JsVlcPlaylist :
    public node::ObjectWrap
{
public:
    enum class PlaybackMode {
        Normal = vlc::mode_normal,
        Loop   = vlc::mode_loop,
        Single = vlc::mode_single,
    };

    static v8::UniquePersistent<v8::Object> create( JsVlcPlayer& player );

    static void initJsApi();
    static void jsCreate( const v8::FunctionCallbackInfo<v8::Value>& args );

private:
    JsVlcPlaylist( v8::Local<v8::Object>& thisObject, JsVlcPlayer* );

private:
    static v8::Persistent<v8::Function> _jsConstructor;

    JsVlcPlayer* _jsPlayer;
};

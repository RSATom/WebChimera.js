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

    unsigned itemCount();
    bool isPlaying();

    int currentItem();
    void setCurrentItem( unsigned );

    unsigned mode();
    void setMode( unsigned );

    int add( const std::string& mrl );
    int addWithOptions( const std::string& mrl, const std::vector<std::string>& options );
    void play();
    bool playItem( unsigned idx );
    void pause();
    void togglePause();
    void stop();
    void next();
    void prev();
    void clear();
    bool removeItem( unsigned idx );
    void advanceItem( unsigned idx, int count );

    v8::Local<v8::Object> items();

private:
    static void jsCreate( const v8::FunctionCallbackInfo<v8::Value>& args );
    JsVlcPlaylist( v8::Local<v8::Object>& thisObject, JsVlcPlayer* );

private:
    static v8::Persistent<v8::Function> _jsConstructor;

    JsVlcPlayer* _jsPlayer;

    v8::UniquePersistent<v8::Object> _jsItems;
};

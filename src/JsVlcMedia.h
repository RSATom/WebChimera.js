#pragma once

#include <v8.h>
#include <node_object_wrap.h>

#include <libvlc_wrapper/vlc_player.h>

class JsVlcPlayer; //#include "JsVlcPlayer.h"

class JsVlcMedia :
    public node::ObjectWrap
{
public:
    static void initJsApi();

    static v8::Local<v8::Object> create( JsVlcPlayer& player,
                                         const vlc::media& media  );
    static void jsCreate( const v8::FunctionCallbackInfo<v8::Value>& args );

    std::string artist();
    std::string genre();
    std::string copyright();
    std::string album();
    std::string trackNumber();
    std::string description();
    std::string rating();
    std::string date();

    std::string URL();
    std::string language();
    std::string nowPlaying();
    std::string publisher();
    std::string encodedBy();
    std::string artworkURL();
    std::string trackID();
    std::string mrl();

    std::string title();
    void setTitle( const std::string& );

    std::string setting();
    void setSetting( const std::string& );

    bool disabled();
    void setDisabled( bool );

private:
    JsVlcMedia( v8::Local<v8::Object>& thisObject,
                JsVlcPlayer*,
                const vlc::media& media );

    std::string meta( libvlc_meta_t e_meta );
    void setMeta( libvlc_meta_t e_meta, const std::string& );

protected:
    vlc::media get_media()
        { return _media; };

private:
    static v8::Persistent<v8::Function> _jsConstructor;

    JsVlcPlayer* _jsPlayer;
    vlc::media _media;
};

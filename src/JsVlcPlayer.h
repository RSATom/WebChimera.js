#pragma once

#include <memory>
#include <deque>
#include <set>

#include <node.h>
#include <node_object_wrap.h>
#include <uv.h>

#include <libvlc_wrapper/vlc_player.h>
#include <libvlc_wrapper/vlc_vmem.h>

#include "VlcVideoOutput.h"

class JsVlcPlayer :
    public node::ObjectWrap,
    private VlcVideoOutput,
    private vlc::media_player_events_callback
{
    enum Callbacks_e {
        CB_FrameSetup = 0,
        CB_FrameReady,
        CB_FrameCleanup,

        CB_MediaPlayerMediaChanged,
        CB_MediaPlayerNothingSpecial,
        CB_MediaPlayerOpening,
        CB_MediaPlayerBuffering,
        CB_MediaPlayerPlaying,
        CB_MediaPlayerPaused,
        CB_MediaPlayerStopped,
        CB_MediaPlayerForward,
        CB_MediaPlayerBackward,
        CB_MediaPlayerEndReached,
        CB_MediaPlayerEncounteredError,

        CB_MediaPlayerTimeChanged,
        CB_MediaPlayerPositionChanged,
        CB_MediaPlayerSeekableChanged,
        CB_MediaPlayerPausableChanged,
        CB_MediaPlayerLengthChanged,

        CB_LogMessage,

        CB_Max,
    };

    static const char* callbackNames[CB_Max];

public:
    static void initJsApi(
        const v8::Local<v8::Object>& exports,
        const v8::Local<v8::Value>& module,
        const v8::Local<v8::Context>& context);

    static void jsPlay(const v8::FunctionCallbackInfo<v8::Value>& args);

    static void getJsCallback(
        v8::Local<v8::String> property,
        const v8::PropertyCallbackInfo<v8::Value>& info,
        Callbacks_e callback);
    static void setJsCallback(
        v8::Local<v8::String> property,
        v8::Local<v8::Value> value,
        const v8::PropertyCallbackInfo<void>& info,
        Callbacks_e callback);

    bool playing();
    double length();
    unsigned state();

    v8::Local<v8::Value> getVideoFrame();
    v8::Local<v8::Object> getEventEmitter();

    unsigned pixelFormat();
    void setPixelFormat(unsigned);

    double position();
    void setPosition(double);

    double time();
    void setTime(double);

    unsigned volume();
    void setVolume(unsigned);

    bool muted();
    void setMuted(bool);

    void play();
    void play(const std::string& mrl);
    void pause();
    void togglePause();
    void stop();
    void toggleMute();

    v8::Local<v8::Object> input();
    v8::Local<v8::Object> audio();
    v8::Local<v8::Object> video();
    v8::Local<v8::Object> subtitles();
    v8::Local<v8::Object> playlist();

    vlc::player& player()
        { return _player; }

    void close();

private:
    struct ContextData;

    static void jsCreate(const v8::FunctionCallbackInfo<v8::Value>& args);
    JsVlcPlayer(
        v8::Local<v8::Object>& thisObject,
        const v8::Local<v8::Array>& vlcOpts,
        ContextData*);
    ~JsVlcPlayer();

    struct AsyncData;
    struct CallbackData;
    struct LibvlcEvent;
    struct LibvlcLogEvent;

    void initLibvlc(const v8::Local<v8::Array>& vlcOpts);

    void handleAsync();

    //could come from worker thread
    void media_player_event(const libvlc_event_t*);

    static void log_event_wrapper(
        void*, int, const libvlc_log_t *, const char *, va_list);
    void log_event(int, const libvlc_log_t *, const char *, va_list);

    void handleLibvlcEvent(const libvlc_event_t&);

    void currentItemEndReached();

    void callCallback(
        Callbacks_e callback,
        std::initializer_list<v8::Local<v8::Value> > list = std::initializer_list<v8::Local<v8::Value> >());

protected:
    void* onFrameSetup(const RV32VideoFrame&) override;
    void* onFrameSetup(const I420VideoFrame&) override;
    void onFrameReady() override;
    void onFrameCleanup() override;

private:
    static v8::Persistent<v8::Function> _jsConstructor;

    ContextData *const _contextData;

    libvlc_instance_t* _libvlc;
    vlc::player _player;

    uv_async_t _async;
    std::mutex _asyncDataGuard;
    std::deque<std::unique_ptr<AsyncData> > _asyncData;

    v8::UniquePersistent<v8::Value> _jsFrameBuffer;

    v8::UniquePersistent<v8::Function> _jsCallbacks[CB_Max];
    v8::UniquePersistent<v8::Object> _jsEventEmitter;

    v8::UniquePersistent<v8::Object> _jsInput;
    v8::UniquePersistent<v8::Object> _jsAudio;
    v8::UniquePersistent<v8::Object> _jsVideo;
    v8::UniquePersistent<v8::Object> _jsSubtitles;
    v8::UniquePersistent<v8::Object> _jsPlaylist;

    uv_timer_t _errorTimer;
};

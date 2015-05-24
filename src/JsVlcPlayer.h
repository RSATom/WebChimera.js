#pragma once

#include <memory>
#include <deque>

#include <v8.h>
#include <node.h>
#include <node_object_wrap.h>

#include <libvlc_wrapper/vlc_player.h>
#include <libvlc_wrapper/vlc_vmem.h>

class JsVlcPlayer :
    public node::ObjectWrap, private vlc::basic_vmem_wrapper
{
    enum Callbacks_e {
        CB_FRAME_SETUP,
        CB_FRAME_READY,
        CB_FRAME_CLEANUP,

        CB_MAX,
    };

public:
    static void initJsApi();
    static void jsCreate( const v8::FunctionCallbackInfo<v8::Value>& args );

    static void jsPlaying( v8::Local<v8::String> property,
                           const v8::PropertyCallbackInfo<v8::Value>& info );
    static void jsLength( v8::Local<v8::String> property,
                          const v8::PropertyCallbackInfo<v8::Value>& info );
    static void jsState( v8::Local<v8::String> property,
                         const v8::PropertyCallbackInfo<v8::Value>& info );

    static void jsPosition( v8::Local<v8::String> property,
                            const v8::PropertyCallbackInfo<v8::Value>& info );
    static void jsSetPosition( v8::Local<v8::String> property,
                               v8::Local<v8::Value> value,
                               const v8::PropertyCallbackInfo<void>& info );

    static void jsTime( v8::Local<v8::String> property,
                        const v8::PropertyCallbackInfo<v8::Value>& info );
    static void jsSetTime( v8::Local<v8::String> property,
                           v8::Local<v8::Value> value,
                           const v8::PropertyCallbackInfo<void>& info );

    static void jsVolume( v8::Local<v8::String> property,
                          const v8::PropertyCallbackInfo<v8::Value>& info );
    static void jsSetVolume( v8::Local<v8::String> property,
                             v8::Local<v8::Value> value,
                             const v8::PropertyCallbackInfo<void>& info );

    static void jsPlay( const v8::FunctionCallbackInfo<v8::Value>& args );
    static void jsPause( const v8::FunctionCallbackInfo<v8::Value>& args );
    static void jsTogglePause( const v8::FunctionCallbackInfo<v8::Value>& args );
    static void jsStop( const v8::FunctionCallbackInfo<v8::Value>& args );
    static void jsToggleMute( const v8::FunctionCallbackInfo<v8::Value>& args );

    static void getJsCallback( v8::Local<v8::String> property,
                               const v8::PropertyCallbackInfo<v8::Value>& info,
                               Callbacks_e callback );
    static void setJsCallback( v8::Local<v8::String> property,
                               v8::Local<v8::Value> value,
                               const v8::PropertyCallbackInfo<void>& info,
                               Callbacks_e callback );

private:
    JsVlcPlayer();
    ~JsVlcPlayer();

    struct AsyncData;
    struct FrameSetupData;
    struct FrameUpdated;
    struct CallbackData;

    void handleAsync();
    void setupBuffer( unsigned width, unsigned height, const std::string& pixelFormat );
    void frameUpdated();

    void callCallback( Callbacks_e callback,
                       std::initializer_list<v8::Local<v8::Value> > list = std::initializer_list<v8::Local<v8::Value> >() );

private:
    unsigned video_format_cb( char* chroma,
                              unsigned* width, unsigned* height,
                              unsigned* pitches, unsigned* lines ) override;
    void video_cleanup_cb() override;

    void* video_lock_cb( void** planes ) override;
    void video_unlock_cb( void* picture, void *const * planes ) override;
    void video_display_cb( void* picture ) override;

private:
    static v8::Persistent<v8::Function> _jsConstructor;

    libvlc_instance_t* _libvlc;
    vlc::player _player;

    uv_async_t _async;
    std::deque<std::shared_ptr<AsyncData> > _asyncData;

    std::vector<char> _tmpFrameBuffer;
    v8::Persistent<v8::Value> _jsFrameBuffer;
    char* _jsRawFrameBuffer;

    v8::Persistent<v8::Function> _jsCallbacks[CB_MAX];
};

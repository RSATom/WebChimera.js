#pragma once

#include <v8.h>
#include <node.h>
#include <node_object_wrap.h>

#include <libvlc_wrapper/vlc_player.h>
#include <libvlc_wrapper/vlc_vmem.h>

class JsVlcPlayer :
    public node::ObjectWrap, private vlc::vmem
{
public:
    static void initJsApi();
    static void jsCreate( const v8::FunctionCallbackInfo<v8::Value>& args );

private:
    static void jsPlay( const v8::FunctionCallbackInfo<v8::Value>& args );
    static void jsStop( const v8::FunctionCallbackInfo<v8::Value>& args );

private:
    JsVlcPlayer( const v8::Local<v8::Function>& renderCallback );
    ~JsVlcPlayer();


private:
    void on_format_setup() override;
    void on_frame_ready( const std::vector<char>* ) override;
    void on_frame_cleanup() override;

private:
    void setupBuffer();
    void frameUpdated();

private:
    static v8::Persistent<v8::Function> _jsConstructor;

    libvlc_instance_t* _libvlc;
    vlc::player _player;

    uv_async_t _formatSetupAsync;
    uv_async_t _frameUpdatedAsync;

    unsigned _frameWidth;
    unsigned _frameHeight;

    v8::Persistent<v8::Object> _jsFrameBuffer;
    unsigned _jsFrameBufferSize;
    char* _jsRawFrameBuffer;

    v8::Persistent<v8::Function> _jsRenderCallback;
};

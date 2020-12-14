#include <node.h>
#include <v8.h>

#include "JsVlcPlayer.h"
#include "NodeTools.h"

extern "C" NODE_MODULE_EXPORT void
NODE_MODULE_INITIALIZER(
    v8::Local<v8::Object> exports,
    v8::Local<v8::Value> module,
    v8::Local<v8::Context> context)
{
    using namespace v8;

    thisModule.Reset(v8::Isolate::GetCurrent(), Local<Object>::Cast(module));

    JsVlcPlayer::initJsApi(exports);
}

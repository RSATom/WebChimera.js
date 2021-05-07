#include <node.h>

#include "JsVlcPlayer.h"
#include "NodeTools.h"

extern "C" NODE_MODULE_EXPORT void
NODE_MODULE_INITIALIZER(
    v8::Local<v8::Object> exports,
    v8::Local<v8::Value> module,
    v8::Local<v8::Context> context)
{
    JsVlcPlayer::initJsApi(exports, module, context);
}

#ifdef __linux__
#include <unistd.h>
#endif

#include <node.h>

#include "JsVlcPlayer.h"
#include "NodeTools.h"

extern "C" NODE_MODULE_EXPORT void
NODE_MODULE_INITIALIZER(
    v8::Local<v8::Object> exports,
    v8::Local<v8::Value> module,
    v8::Local<v8::Context> context)
{
#if defined(__linux__) && !defined(NDEBUG) && 0
    printf("WebChimera.js process pid: %ld\n", (long)getpid());
    int i = 0;
    do {
        usleep(100000);  // sleep for 0.1 seconds
    } while(!i);
#endif

    JsVlcPlayer::initJsApi(exports, module, context);
}

#include <node.h>
#include <v8.h>

#include "JsVlcPlayer.h"

void Init( v8::Handle<v8::Object> exports )
{
    JsVlcPlayer::initJsApi();

    NODE_SET_METHOD( exports, "createPlayer", JsVlcPlayer::jsCreate );
}

NODE_MODULE( WebChimera, Init )

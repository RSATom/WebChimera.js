#include <node.h>
#include <v8.h>

#include "JsVlcPlayer.h"
#include "NodeTools.h"

void Init( v8::Handle<v8::Object> exports, v8::Handle<v8::Object> module )
{
    thisModule.Reset( v8::Isolate::GetCurrent(), module );

    JsVlcPlayer::initJsApi( exports );
}

NODE_MODULE( WebChimera, Init )

#include "JsVlcInput.h"

#include "NodeTools.h"
#include "JsVlcPlayer.h"
#include "JsVlcDeinterlace.h"

v8::Persistent<v8::Function> JsVlcInput::_jsConstructor;

void JsVlcInput::initJsApi()
{
    JsVlcDeinterlace::initJsApi();

    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<FunctionTemplate> constructorTemplate = FunctionTemplate::New( isolate, jsCreate );
    constructorTemplate->SetClassName(
        String::NewFromUtf8( isolate, "VlcInput", v8::String::kInternalizedString ) );

    Local<ObjectTemplate> protoTemplate = constructorTemplate->PrototypeTemplate();
    Local<ObjectTemplate> instanceTemplate = constructorTemplate->InstanceTemplate();
    instanceTemplate->SetInternalFieldCount( 1 );

    SET_RO_PROPERTY( instanceTemplate, "length", &JsVlcInput::length );
    SET_RO_PROPERTY( instanceTemplate, "fps", &JsVlcInput::fps );
    SET_RO_PROPERTY( instanceTemplate, "state", &JsVlcInput::state );

    SET_RW_PROPERTY( instanceTemplate, "position",
                     &JsVlcInput::position,
                     &JsVlcInput::setPosition );
    SET_RW_PROPERTY( instanceTemplate, "time",
                     &JsVlcInput::time,
                     &JsVlcInput::setTime );
    SET_RW_PROPERTY( instanceTemplate, "rate",
                     &JsVlcInput::rate,
                     &JsVlcInput::setRate );

    Local<Function> constructor = constructorTemplate->GetFunction();
    _jsConstructor.Reset( isolate, constructor );
}

v8::UniquePersistent<v8::Object> JsVlcInput::create( JsVlcPlayer& player )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Function> constructor =
        Local<Function>::New( isolate, _jsConstructor );

    Local<Value> argv[] = { player.handle() };

    return { isolate, constructor->NewInstance( sizeof( argv ) / sizeof( argv[0] ), argv ) };
}

void JsVlcInput::jsCreate( const v8::FunctionCallbackInfo<v8::Value>& args )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Object> thisObject = args.Holder();
    if( args.IsConstructCall() && thisObject->InternalFieldCount() > 0 ) {
        JsVlcPlayer* jsPlayer =
            ObjectWrap::Unwrap<JsVlcPlayer>( Handle<Object>::Cast( args[0] ) );
        if( jsPlayer ) {
            JsVlcInput* jsPlaylist = new JsVlcInput( thisObject, jsPlayer );
            args.GetReturnValue().Set( thisObject );
        }
    } else {
        Local<Function> constructor =
            Local<Function>::New( isolate, _jsConstructor );
        Local<Value> argv[] = { args[0] };
        args.GetReturnValue().Set(
            constructor->NewInstance( sizeof( argv ) / sizeof( argv[0] ), argv ) );
    }
}

JsVlcInput::JsVlcInput( v8::Local<v8::Object>& thisObject, JsVlcPlayer* jsPlayer ) :
    _jsPlayer( jsPlayer )
{
    Wrap( thisObject );
}

double JsVlcInput::length()
{
    return static_cast<double>( _jsPlayer->player().get_length() );
}

double JsVlcInput::fps()
{
    return _jsPlayer->player().get_fps();
}

unsigned JsVlcInput::state()
{
    return _jsPlayer->player().get_state();
}

double JsVlcInput::position()
{
    return _jsPlayer->player().get_position();
}

void JsVlcInput::setPosition( double position )
{
    _jsPlayer->player().set_position( static_cast<float>( position ) );
}

double JsVlcInput::time()
{
    return static_cast<double>( _jsPlayer->player().get_time() );
}

void JsVlcInput::setTime( double time )
{
    return _jsPlayer->player().set_time( static_cast<libvlc_time_t>( time ) );
}

double JsVlcInput::rate()
{
    return _jsPlayer->player().get_rate();
}

void JsVlcInput::setRate( double rate )
{
    _jsPlayer->player().set_rate( static_cast<float>( rate ) );
}


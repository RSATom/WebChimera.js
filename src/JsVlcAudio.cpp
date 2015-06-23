#include "JsVlcAudio.h"

#include "NodeTools.h"
#include "JsVlcPlayer.h"

v8::Persistent<v8::Function> JsVlcAudio::_jsConstructor;

void JsVlcAudio::initJsApi()
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<FunctionTemplate> constructorTemplate = FunctionTemplate::New( isolate, jsCreate );
    constructorTemplate->SetClassName(
        String::NewFromUtf8( isolate, "VlcVideo", v8::String::kInternalizedString ) );

    Local<ObjectTemplate> protoTemplate = constructorTemplate->PrototypeTemplate();
    Local<ObjectTemplate> instanceTemplate = constructorTemplate->InstanceTemplate();
    instanceTemplate->SetInternalFieldCount( 1 );

    protoTemplate->Set( String::NewFromUtf8( isolate, "Error", v8::String::kInternalizedString ),
                        Integer::New( isolate, libvlc_AudioChannel_Error ),
                        static_cast<v8::PropertyAttribute>( ReadOnly | DontDelete ) );
    protoTemplate->Set( String::NewFromUtf8( isolate, "Stereo", v8::String::kInternalizedString ),
                        Integer::New( isolate, libvlc_AudioChannel_Stereo ),
                        static_cast<v8::PropertyAttribute>( ReadOnly | DontDelete ) );
    protoTemplate->Set( String::NewFromUtf8( isolate, "ReverseStereo", v8::String::kInternalizedString ),
                        Integer::New( isolate, libvlc_AudioChannel_RStereo ),
                        static_cast<v8::PropertyAttribute>( ReadOnly | DontDelete ) );
    protoTemplate->Set( String::NewFromUtf8( isolate, "Left", v8::String::kInternalizedString ),
                        Integer::New( isolate, libvlc_AudioChannel_Left ),
                        static_cast<v8::PropertyAttribute>( ReadOnly | DontDelete ) );
    protoTemplate->Set( String::NewFromUtf8( isolate, "Right", v8::String::kInternalizedString ),
                        Integer::New( isolate, libvlc_AudioChannel_Right ),
                        static_cast<v8::PropertyAttribute>( ReadOnly | DontDelete ) );
    protoTemplate->Set( String::NewFromUtf8( isolate, "Dolby", v8::String::kInternalizedString ),
                        Integer::New( isolate, libvlc_AudioChannel_Dolbys ),
                        static_cast<v8::PropertyAttribute>( ReadOnly | DontDelete ) );

    SET_RO_INDEXED_PROPERTY( instanceTemplate, &JsVlcAudio::description );

    SET_RO_PROPERTY( instanceTemplate, "count", &JsVlcAudio::count );

    SET_RW_PROPERTY( instanceTemplate, "track", &JsVlcAudio::track, &JsVlcAudio::setTrack );
    SET_RW_PROPERTY( instanceTemplate, "mute", &JsVlcAudio::muted, &JsVlcAudio::setMuted );
    SET_RW_PROPERTY( instanceTemplate, "volume", &JsVlcAudio::volume, &JsVlcAudio::setVolume );
    SET_RW_PROPERTY( instanceTemplate, "channel", &JsVlcAudio::channel, &JsVlcAudio::setChannel );
    SET_RW_PROPERTY( instanceTemplate, "delay", &JsVlcAudio::delay, &JsVlcAudio::setDelay );

    SET_METHOD( constructorTemplate, "toggleMute", &JsVlcAudio::toggleMute );

    Local<Function> constructor = constructorTemplate->GetFunction();
    _jsConstructor.Reset( isolate, constructor );
}

v8::UniquePersistent<v8::Object> JsVlcAudio::create( JsVlcPlayer& player )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Function> constructor =
        Local<Function>::New( isolate, _jsConstructor );

    Local<Value> argv[] = { player.handle() };

    return { isolate, constructor->NewInstance( sizeof( argv ) / sizeof( argv[0] ), argv ) };
}

void JsVlcAudio::jsCreate( const v8::FunctionCallbackInfo<v8::Value>& args )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Object> thisObject = args.Holder();
    if( args.IsConstructCall() && thisObject->InternalFieldCount() > 0 ) {
        JsVlcPlayer* jsPlayer =
            ObjectWrap::Unwrap<JsVlcPlayer>( Handle<Object>::Cast( args[0] ) );
        if( jsPlayer ) {
            JsVlcAudio* jsPlaylist = new JsVlcAudio( thisObject, jsPlayer );
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

JsVlcAudio::JsVlcAudio( v8::Local<v8::Object>& thisObject, JsVlcPlayer* jsPlayer ) :
    _jsPlayer( jsPlayer )
{
    Wrap( thisObject );
}

std::string JsVlcAudio::description( uint32_t index )
{
    vlc_player& p = _jsPlayer->player();

    std::string name;

    libvlc_track_description_t* rootTrackDesc =
        libvlc_audio_get_track_description( p.get_mp() );
    if( !rootTrackDesc )
        return name;

    unsigned count = _jsPlayer->player().audio().track_count();
    if( count && index < count ) {
        libvlc_track_description_t* trackDesc = rootTrackDesc;
        for( ; index && trackDesc; --index ){
            trackDesc = trackDesc->p_next;
        }

        if ( trackDesc && trackDesc->psz_name ) {
            name = trackDesc->psz_name;
        }
    }
    libvlc_track_description_list_release( rootTrackDesc );

    return name;
}

unsigned JsVlcAudio::count()
{
    return _jsPlayer->player().audio().track_count();
}

int JsVlcAudio::track()
{
    return _jsPlayer->player().audio().get_track();
}

void JsVlcAudio::setTrack( int track )
{
    _jsPlayer->player().audio().set_track( track );
}

int JsVlcAudio::delay()
{
    return static_cast<int>( _jsPlayer->player().audio().get_delay() );
}

void JsVlcAudio::setDelay( int delay )
{
    _jsPlayer->player().audio().set_delay( delay );
}

bool JsVlcAudio::muted()
{
    return _jsPlayer->player().audio().is_muted();
}

void JsVlcAudio::setMuted( bool muted )
{
    _jsPlayer->player().audio().set_mute( muted );
}

unsigned JsVlcAudio::volume()
{
    return _jsPlayer->player().audio().get_volume();
}

void JsVlcAudio::setVolume( unsigned volume )
{
    _jsPlayer->player().audio().set_volume( volume );
}

int JsVlcAudio::channel()
{
    return _jsPlayer->player().audio().get_channel();
}

void JsVlcAudio::setChannel( unsigned channel )
{
    _jsPlayer->player().audio().set_channel( (libvlc_audio_output_channel_t) channel );
}

void JsVlcAudio::toggleMute()
{
    _jsPlayer->player().audio().toggle_mute();
}

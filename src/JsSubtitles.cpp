#include "JsSubtitles.h"

#include "NodeTools.h"
#include "JsVlcPlayer.h"

v8::Persistent<v8::Function> JsSubtitles::_jsConstructor;

void JsSubtitles::initJsApi()
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<FunctionTemplate> constructorTemplate = FunctionTemplate::New( isolate, jsCreate );
    constructorTemplate->SetClassName(
        String::NewFromUtf8( isolate, "VlcPlaylistItems", v8::String::kInternalizedString ) );

    Local<ObjectTemplate> protoTemplate = constructorTemplate->PrototypeTemplate();
    Local<ObjectTemplate> instanceTemplate = constructorTemplate->InstanceTemplate();
    instanceTemplate->SetInternalFieldCount( 1 );

    SET_RO_INDEXED_PROPERTY( instanceTemplate, &JsSubtitles::description );

    SET_RO_PROPERTY( instanceTemplate, "count", &JsSubtitles::count );

    SET_RW_PROPERTY( instanceTemplate, "track", &JsSubtitles::track, &JsSubtitles::setTrack );
    SET_RW_PROPERTY( instanceTemplate, "delay", &JsSubtitles::delay, &JsSubtitles::setDelay );

    Local<Function> constructor = constructorTemplate->GetFunction();
    _jsConstructor.Reset( isolate, constructor );
}

v8::UniquePersistent<v8::Object> JsSubtitles::create( JsVlcPlayer& player )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Function> constructor =
        Local<Function>::New( isolate, _jsConstructor );

    Local<Value> argv[] = { player.handle() };

    return { isolate, constructor->NewInstance( sizeof( argv ) / sizeof( argv[0] ), argv ) };
}

void JsSubtitles::jsCreate( const v8::FunctionCallbackInfo<v8::Value>& args )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Object> thisObject = args.Holder();
    if( args.IsConstructCall() && thisObject->InternalFieldCount() > 0 ) {
        JsVlcPlayer* jsPlayer =
            ObjectWrap::Unwrap<JsVlcPlayer>( Handle<Object>::Cast( args[0] ) );
        if( jsPlayer ) {
            JsSubtitles* jsPlaylist = new JsSubtitles( thisObject, jsPlayer );
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

JsSubtitles::JsSubtitles( v8::Local<v8::Object>& thisObject, JsVlcPlayer* jsPlayer ) :
    _jsPlayer( jsPlayer )
{
    Wrap( thisObject );
}

std::string JsSubtitles::description( uint32_t index )
{
    vlc_player& p = _jsPlayer->player();

    std::string name;

    libvlc_track_description_t* rootDesc =
        libvlc_video_get_spu_description( p.get_mp() );
    if( !rootDesc )
        return name;

    unsigned count = libvlc_video_get_spu_count( p.get_mp() );
    if( count && index < count ) {
        libvlc_track_description_t* desc = rootDesc;
        for( ; index && desc; --index ){
            desc = desc->p_next;
        }

        if ( desc && desc->psz_name ) {
            name = desc->psz_name;
        }
    }
    libvlc_track_description_list_release( rootDesc );

    return name;
}

unsigned JsSubtitles::count()
{
    return _jsPlayer->player().subtitles().track_count();
}

int JsSubtitles::track()
{
    return _jsPlayer->player().subtitles().get_track();
}

void JsSubtitles::setTrack( int track )
{
    return _jsPlayer->player().subtitles().set_track( track );
}

int JsSubtitles::delay()
{
    return static_cast<int>( _jsPlayer->player().subtitles().get_delay() );
}

void JsSubtitles::setDelay( int delay )
{
    _jsPlayer->player().subtitles().set_delay( delay );
}

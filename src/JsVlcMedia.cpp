#include "JsVlcMedia.h"

#include "NodeTools.h"
#include "JsVlcPlayer.h"

v8::Persistent<v8::Function> JsVlcMedia::_jsConstructor;

void JsVlcMedia::initJsApi()
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<FunctionTemplate> constructorTemplate = FunctionTemplate::New( isolate, jsCreate );
    constructorTemplate->SetClassName( String::NewFromUtf8( isolate, "JsVlcMedia", v8::String::kInternalizedString ) );

    Local<ObjectTemplate> protoTemplate = constructorTemplate->PrototypeTemplate();
    Local<ObjectTemplate> instanceTemplate = constructorTemplate->InstanceTemplate();
    instanceTemplate->SetInternalFieldCount( 1 );

    SET_RO_PROPERTY( instanceTemplate, "artist", &JsVlcMedia::artist );
    SET_RO_PROPERTY( instanceTemplate, "genre", &JsVlcMedia::genre );
    SET_RO_PROPERTY( instanceTemplate, "copyright", &JsVlcMedia::copyright );
    SET_RO_PROPERTY( instanceTemplate, "album", &JsVlcMedia::album );
    SET_RO_PROPERTY( instanceTemplate, "trackNumber", &JsVlcMedia::trackNumber );
    SET_RO_PROPERTY( instanceTemplate, "description", &JsVlcMedia::description );
    SET_RO_PROPERTY( instanceTemplate, "rating", &JsVlcMedia::rating );
    SET_RO_PROPERTY( instanceTemplate, "date", &JsVlcMedia::date );
    SET_RO_PROPERTY( instanceTemplate, "URL", &JsVlcMedia::URL );
    SET_RO_PROPERTY( instanceTemplate, "language", &JsVlcMedia::language );
    SET_RO_PROPERTY( instanceTemplate, "nowPlaying", &JsVlcMedia::nowPlaying );
    SET_RO_PROPERTY( instanceTemplate, "publisher", &JsVlcMedia::publisher );
    SET_RO_PROPERTY( instanceTemplate, "encodedBy", &JsVlcMedia::encodedBy );
    SET_RO_PROPERTY( instanceTemplate, "artworkURL", &JsVlcMedia::artworkURL );
    SET_RO_PROPERTY( instanceTemplate, "trackID", &JsVlcMedia::trackID );
    SET_RO_PROPERTY( instanceTemplate, "mrl", &JsVlcMedia::mrl );

    SET_RW_PROPERTY( instanceTemplate, "title",
                     &JsVlcMedia::title,
                     &JsVlcMedia::setTitle );
    SET_RW_PROPERTY( instanceTemplate, "setting",
                     &JsVlcMedia::setting,
                     &JsVlcMedia::setSetting );
    SET_RW_PROPERTY( instanceTemplate, "disabled",
                     &JsVlcMedia::disabled,
                     &JsVlcMedia::setDisabled );

    Local<Function> constructor = constructorTemplate->GetFunction();
    _jsConstructor.Reset( isolate, constructor );
}

v8::Local<v8::Object> JsVlcMedia::create( JsVlcPlayer& player,
                                          const vlc::media& media )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    EscapableHandleScope scope( isolate );

    Local<Function> constructor =
        Local<Function>::New( isolate, _jsConstructor );

    Local<Value> argv[] = { player.handle(), External::New( isolate, const_cast<vlc::media*>( &media ) ) };

    return scope.Escape( constructor->NewInstance( sizeof( argv ) / sizeof( argv[0] ), argv ) );
}

void JsVlcMedia::jsCreate( const v8::FunctionCallbackInfo<v8::Value>& args )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Object> thisObject = args.Holder();
    if( args.IsConstructCall() && thisObject->InternalFieldCount() > 0 ) {
        JsVlcPlayer* jsPlayer =
            ObjectWrap::Unwrap<JsVlcPlayer>( Local<Object>::Cast( args[0] ) );

        const vlc::media* media =
            static_cast<const vlc::media*>( Local<External>::Cast( args[1] )->Value() );

        if( jsPlayer && media ) {
            JsVlcMedia* jsPlaylist = new JsVlcMedia( thisObject, jsPlayer, *media );
            args.GetReturnValue().Set( thisObject );
        }
    } else {
        Local<Function> constructor =
            Local<Function>::New( isolate, _jsConstructor );
        Local<Value> argv[] = { args[0], args[1] };
        args.GetReturnValue().Set(
            constructor->NewInstance( sizeof( argv ) / sizeof( argv[0] ), argv ) );
    }
}

JsVlcMedia::JsVlcMedia( v8::Local<v8::Object>& thisObject,
                        JsVlcPlayer* jsPlayer,
                        const vlc::media& media ) :
    _jsPlayer( jsPlayer ), _media( media )
{
    Wrap( thisObject );
}

std::string JsVlcMedia::meta( libvlc_meta_t e_meta )
{
    return get_media().meta( e_meta );
}

void JsVlcMedia::setMeta( libvlc_meta_t e_meta, const std::string& meta )
{
    return get_media().set_meta( e_meta, meta );
}

std::string JsVlcMedia::artist()
{
    return meta( libvlc_meta_Artist );
}

std::string JsVlcMedia::genre()
{
    return meta( libvlc_meta_Genre );
}

std::string JsVlcMedia::copyright()
{
    return meta( libvlc_meta_Copyright );
}

std::string JsVlcMedia::album()
{
    return meta( libvlc_meta_Album );
}

std::string JsVlcMedia::trackNumber()
{
    return meta( libvlc_meta_TrackNumber );
}

std::string JsVlcMedia::description()
{
    return meta( libvlc_meta_Description );
}

std::string JsVlcMedia::rating()
{
    return meta( libvlc_meta_Rating );
}

std::string JsVlcMedia::date()
{
    return meta( libvlc_meta_Date );
}

std::string JsVlcMedia::URL()
{
    return meta( libvlc_meta_URL );
}

std::string JsVlcMedia::language()
{
    return meta( libvlc_meta_Language );
}

std::string JsVlcMedia::nowPlaying()
{
    return meta( libvlc_meta_NowPlaying );
}

std::string JsVlcMedia::publisher()
{
    return meta( libvlc_meta_Publisher );
}

std::string JsVlcMedia::encodedBy()
{
    return meta( libvlc_meta_EncodedBy );
}

std::string JsVlcMedia::artworkURL()
{
    return meta( libvlc_meta_ArtworkURL );
}

std::string JsVlcMedia::trackID()
{
    return meta( libvlc_meta_TrackID );
}

std::string JsVlcMedia::mrl()
{
    return get_media().mrl();
}

std::string JsVlcMedia::title()
{
    return meta( libvlc_meta_Title );
}

void JsVlcMedia::setTitle( const std::string& title )
{
    setMeta( libvlc_meta_Title, title );
}

std::string JsVlcMedia::setting()
{
    vlc_player& p = _jsPlayer->player();

    int idx = p.find_media_index( get_media() );
    if( idx >= 0 ) {
        return p.get_item_data( idx );
    }

    return std::string();
}

void JsVlcMedia::setSetting( const std::string& setting )
{
    vlc_player& p = _jsPlayer->player();

    int idx = p.find_media_index( get_media() );
    if( idx >= 0 ) {
        p.set_item_data( idx, setting );
    }
}

bool JsVlcMedia::disabled()
{
    vlc_player& p = _jsPlayer->player();

    int idx = p.find_media_index( get_media() );
    return idx < 0 ? false : p.is_item_disabled( idx );
}

void JsVlcMedia::setDisabled( bool disabled )
{
    vlc_player& p = _jsPlayer->player();

    int idx = p.find_media_index( get_media() );
    if( idx >= 0 ) {
        p.disable_item( idx, disabled );
    }
}


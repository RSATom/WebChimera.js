#include "JsVlcPlaylist.h"

#include "NodeTools.h"
#include "JsVlcPlayer.h"
#include "JsVlcPlaylistItems.h"

v8::Persistent<v8::Function> JsVlcPlaylist::_jsConstructor;

void JsVlcPlaylist::initJsApi()
{
    JsVlcPlaylistItems::initJsApi();

    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<FunctionTemplate> constructorTemplate = FunctionTemplate::New( isolate, jsCreate );
    constructorTemplate->SetClassName( String::NewFromUtf8( isolate, "VlcPlaylist", v8::String::kInternalizedString ) );

    Local<ObjectTemplate> protoTemplate = constructorTemplate->PrototypeTemplate();
    Local<ObjectTemplate> instanceTemplate = constructorTemplate->InstanceTemplate();
    instanceTemplate->SetInternalFieldCount( 1 );

    protoTemplate->Set( String::NewFromUtf8( isolate, "Normal", v8::String::kInternalizedString ),
                        Integer::New( isolate, static_cast<int>( PlaybackMode::Normal ) ),
                        static_cast<v8::PropertyAttribute>( ReadOnly | DontDelete ) );
    protoTemplate->Set( String::NewFromUtf8( isolate, "Loop", v8::String::kInternalizedString ),
                        Integer::New( isolate, static_cast<int>( PlaybackMode::Loop ) ),
                        static_cast<v8::PropertyAttribute>( ReadOnly | DontDelete ) );
    protoTemplate->Set( String::NewFromUtf8( isolate, "Single", v8::String::kInternalizedString ),
                        Integer::New( isolate, static_cast<int>( PlaybackMode::Single ) ),
                        static_cast<v8::PropertyAttribute>( ReadOnly | DontDelete ) );

    SET_RO_PROPERTY( instanceTemplate, "itemCount", &JsVlcPlaylist::itemCount );
    SET_RO_PROPERTY( instanceTemplate, "isPlaying", &JsVlcPlaylist::isPlaying );
    SET_RO_PROPERTY( instanceTemplate, "items", &JsVlcPlaylist::items );

    SET_RW_PROPERTY( instanceTemplate, "currentItem", &JsVlcPlaylist::currentItem, &JsVlcPlaylist::setCurrentItem );
    SET_RW_PROPERTY( instanceTemplate, "mode", &JsVlcPlaylist::mode, &JsVlcPlaylist::setMode );

    SET_METHOD( constructorTemplate, "add", &JsVlcPlaylist::add );
    SET_METHOD( constructorTemplate, "addWithOptions", &JsVlcPlaylist::addWithOptions );
    SET_METHOD( constructorTemplate, "play", &JsVlcPlaylist::play );
    SET_METHOD( constructorTemplate, "playItem", &JsVlcPlaylist::playItem );
    SET_METHOD( constructorTemplate, "pause", &JsVlcPlaylist::pause );
    SET_METHOD( constructorTemplate, "togglePause", &JsVlcPlaylist::togglePause );
    SET_METHOD( constructorTemplate, "stop",  &JsVlcPlaylist::stop );
    SET_METHOD( constructorTemplate, "next",  &JsVlcPlaylist::next );
    SET_METHOD( constructorTemplate, "prev",  &JsVlcPlaylist::prev );
    SET_METHOD( constructorTemplate, "clear",  &JsVlcPlaylist::clear );
    SET_METHOD( constructorTemplate, "removeItem",  &JsVlcPlaylist::removeItem );
    SET_METHOD( constructorTemplate, "advanceItem",  &JsVlcPlaylist::advanceItem );

    Local<Function> constructor = constructorTemplate->GetFunction();
    _jsConstructor.Reset( isolate, constructor );
}

v8::UniquePersistent<v8::Object> JsVlcPlaylist::create( JsVlcPlayer& player )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Function> constructor =
        Local<Function>::New( isolate, _jsConstructor );

    Local<Value> argv[] = { player.handle() };

    return { isolate, constructor->NewInstance( sizeof( argv ) / sizeof( argv[0] ), argv ) };
}

void JsVlcPlaylist::jsCreate( const v8::FunctionCallbackInfo<v8::Value>& args )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    Local<Object> thisObject = args.Holder();
    if( args.IsConstructCall() && thisObject->InternalFieldCount() > 0 ) {
        JsVlcPlayer* jsPlayer =
            ObjectWrap::Unwrap<JsVlcPlayer>( Handle<Object>::Cast( args[0] ) );
        if( jsPlayer ) {
            JsVlcPlaylist* jsPlaylist = new JsVlcPlaylist( thisObject, jsPlayer );
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

JsVlcPlaylist::JsVlcPlaylist( v8::Local<v8::Object>& thisObject, JsVlcPlayer* jsPlayer ) :
    _jsPlayer( jsPlayer )
{
    Wrap( thisObject );

    _jsItems = JsVlcPlaylistItems::create( *jsPlayer );
}

unsigned JsVlcPlaylist::itemCount()
{
    return _jsPlayer->player().item_count();
}

bool JsVlcPlaylist::isPlaying()
{
    return _jsPlayer->player().is_playing();
}

unsigned JsVlcPlaylist::mode()
{
    return static_cast<unsigned>( _jsPlayer->player().get_playback_mode() );
}

void JsVlcPlaylist::setMode( unsigned  mode )
{
    vlc::player& p = _jsPlayer->player();

    switch( mode ) {
        case static_cast<unsigned>( PlaybackMode::Normal ):
           p.set_playback_mode( vlc::mode_normal );
            break;
        case static_cast<unsigned>( PlaybackMode::Loop ):
            p.set_playback_mode( vlc::mode_loop );
            break;
        case static_cast<unsigned>( PlaybackMode::Single ):
            p.set_playback_mode( vlc::mode_single );
            break;
    }
}

int JsVlcPlaylist::currentItem()
{
    return _jsPlayer->player().current_item();
}

void JsVlcPlaylist::setCurrentItem( unsigned idx )
{
    _jsPlayer->player().set_current( idx );
}

int JsVlcPlaylist::add( const std::string& mrl )
{
    return _jsPlayer->player().add_media( mrl.c_str() );
}

int JsVlcPlaylist::addWithOptions( const std::string& mrl,
                                   const std::vector<std::string>& options )
{
    std::vector<const char*> trusted_opts;
    trusted_opts.reserve( options.size() );

    for( const std::string& opt: options ) {
        trusted_opts.push_back( opt.c_str() );
    }

    return _jsPlayer->player().add_media( mrl.c_str(),
                                          0, nullptr,
                                          trusted_opts.size(), trusted_opts.data() );
}

void JsVlcPlaylist::play()
{
    _jsPlayer->player().play();
}

bool JsVlcPlaylist::playItem( unsigned idx )
{
    return _jsPlayer->player().play( idx );
}

void JsVlcPlaylist::pause()
{
    _jsPlayer->player().pause();
}

void JsVlcPlaylist::togglePause()
{
    _jsPlayer->player().togglePause();
}

void JsVlcPlaylist::stop()
{
    _jsPlayer->player().stop();
}

void JsVlcPlaylist::next()
{
    _jsPlayer->player().next();
}

void JsVlcPlaylist::prev()
{
    _jsPlayer->player().prev();
}

void JsVlcPlaylist::clear()
{
    _jsPlayer->player().clear_items();
}

bool JsVlcPlaylist::removeItem( unsigned idx )
{
    return _jsPlayer->player().delete_item( idx );
}

void JsVlcPlaylist::advanceItem( unsigned idx, int count )
{
    _jsPlayer->player().advance_item( idx, count );
}

v8::Local<v8::Object> JsVlcPlaylist::items()
{
    return v8::Local<v8::Object>::New( v8::Isolate::GetCurrent(), _jsItems );
}

#include "NodeTools.h"

template<>
std::vector<std::string> FromJsValue<std::vector<std::string> >( const v8::Local<v8::Value>& value )
{
    std::vector<std::string> result;

    if( value->IsArray() ) {
        v8::Local<v8::Array> jsArray = v8::Local<v8::Array>::Cast( value );

        for( unsigned i = 0 ; i < jsArray->Length(); ++i ) {
            v8::String::Utf8Value item( jsArray->Get(i)->ToString() );
            if( item.length() ) {
                result.emplace( result.end(), *item );
            }
        }
    }

    return std::move( result );
}

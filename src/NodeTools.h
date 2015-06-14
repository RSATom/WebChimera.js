#pragma once

#include "Tools.h"

template<typename T>
T FromJsValue( v8::Local<v8::Value>& value )
{
    return typename T::Cast( value );
}

template<>
inline v8::Local<v8::Value> FromJsValue<v8::Local<v8::Value> >( v8::Local<v8::Value>& value )
{
    return value;
}

template<>
inline std::string FromJsValue<std::string>( v8::Local<v8::Value>& value )
{
    v8::String::Utf8Value str( value->ToString() );

    return *str;
}

template<>
inline unsigned FromJsValue<unsigned>( v8::Local<v8::Value>& value )
{
    return static_cast<unsigned>( v8::Local<v8::Integer>::Cast( value )->Value() );
}

template<>
inline int FromJsValue<int>( v8::Local<v8::Value>& value )
{
    return static_cast<int>( v8::Local<v8::Integer>::Cast( value )->Value() );
}

template<>
std::vector<std::string> FromJsValue<std::vector<std::string> >( v8::Local<v8::Value>& value )
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

inline v8::Local<v8::Value> ToJsValue( v8::Local<v8::Value>& value )
{
    return value;
}

inline v8::Local<v8::Value> ToJsValue( int value )
{
    return v8::Integer::New( v8::Isolate::GetCurrent(), value );
}

inline v8::Local<v8::Value> ToJsValue( unsigned value )
{
    return v8::Integer::New( v8::Isolate::GetCurrent(), value );
}

template<typename C, typename ... A, unsigned ... I >
void CallMethod( void ( C::* method) ( A ... ),
                 const v8::FunctionCallbackInfo<v8::Value>& info, StaticSequence<I ...> )
{
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope( isolate );

    C* instance = node::ObjectWrap::Unwrap<C>( info.Holder() );

    ( instance->*method ) ( FromJsValue<std::remove_const<std::remove_reference<A>::type>::type >( info[I] ) ... );
};

template<typename R, typename C, typename ... A, unsigned ... I >
void CallMethod( R ( C::* method) ( A ... ),
                 const v8::FunctionCallbackInfo<v8::Value>& info, StaticSequence<I ...> )
{
    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope( isolate );

    C* instance = node::ObjectWrap::Unwrap<C>( info.Holder() );

    info.GetReturnValue().Set(
        ToJsValue(
            ( instance->*method ) (
                FromJsValue<std::remove_const<std::remove_reference<A>::type>::type >( info[I] ) ... ) ) );
};

template<typename R, typename C, typename ... A>
void CallMethod( R ( C::* method ) ( A ... ), const v8::FunctionCallbackInfo<v8::Value>& info )
{
    return CallMethod( method, info, MakeStaticSequence<sizeof ... ( A )>::Sequence() );
}

template<typename R, typename C>
void GetPropertyValue( R ( C::* getter ) (), const v8::PropertyCallbackInfo<v8::Value>& info )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    C* instance = node::ObjectWrap::Unwrap<C>( info.Holder() );

    info.GetReturnValue().Set( ToJsValue( ( instance->*getter ) () ) );
}

template<typename C, typename V>
void SetPropertyValue( void ( C::* setter ) (V),
                       v8::Local<v8::Value> value,
                       const v8::PropertyCallbackInfo<void>& info )
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope( isolate );

    C* instance = node::ObjectWrap::Unwrap<C>( info.Holder() );

    typedef std::remove_const<std::remove_reference<V>::type>::type cleanPropType;
    ( instance->*setter ) ( FromJsValue<cleanPropType>( value ) );
}

#define SET_METHOD( funTemplate, name, member )                  \
    NODE_SET_PROTOTYPE_METHOD( funTemplate, name,                \
        [] ( const v8::FunctionCallbackInfo<v8::Value>& info ) { \
            CallMethod( member, info );                          \
        }                                                        \
    )

#define SET_RO_PROPERTY( objTemplate, name, member )                              \
    objTemplate->SetAccessor( String::NewFromUtf8( Isolate::GetCurrent(), name ), \
        [] ( v8::Local<v8::String> property,                                      \
             const v8::PropertyCallbackInfo<v8::Value>& info )                    \
        {                                                                         \
            GetPropertyValue( member, info );                                     \
        }                                                                         \
    )

#define SET_RW_PROPERTY( objTemplate, name, getter, setter )                      \
    objTemplate->SetAccessor( String::NewFromUtf8( Isolate::GetCurrent(), name ), \
        [] ( v8::Local<v8::String> /*property*/,                                  \
             const v8::PropertyCallbackInfo<v8::Value>& info )                    \
        {                                                                         \
            GetPropertyValue( getter, info );                                     \
        },                                                                        \
        [] ( v8::Local<v8::String> /*property*/,                                  \
             v8::Local<v8::Value> value,                                          \
             const v8::PropertyCallbackInfo<void>& info )                         \
        {                                                                         \
            SetPropertyValue( setter, value, info );                              \
        }                                                                         \
    )

#include "NodeTools.h"


template<>
std::vector<std::string> FromJsValue<std::vector<std::string> >(
    const v8::Local<v8::Value>& value)
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    Local<Context> context = isolate->GetCurrentContext();

    std::vector<std::string> result;

    if(value->IsArray()) {
        Local<Array> jsArray = Local<Array>::Cast(value);

        for(unsigned i = 0 ; i < jsArray->Length(); ++i) {
            String::Utf8Value item(isolate, jsArray->Get(context, i).ToLocalChecked());
            if(item.length())
                result.emplace(result.end(), *item);
        }
    }

    return std::move(result);
}

static v8::Local<v8::Function> RequireFunc(const v8::Local<v8::Object>& thisModule)
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    Local<Context> context = isolate->GetCurrentContext();

    Local<String > name =
        String::NewFromUtf8(
            isolate,
            "require",
            NewStringType::kInternalized).ToLocalChecked();

    return Local<Function>::Cast(thisModule->Get(context, name).ToLocalChecked());
}

v8::Local<v8::Object> Require(
    const v8::Local<v8::Object>& thisModule,
    const char* module)
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    Local<Context> context = isolate->GetCurrentContext();
    Local<Object> global = context->Global();

    Local<Value> argv[] =
        { String::NewFromUtf8(isolate, module, NewStringType::kInternalized).ToLocalChecked() };

    return Local<Object>::Cast(RequireFunc(thisModule)->Call(context, global, 1, argv).ToLocalChecked());
}

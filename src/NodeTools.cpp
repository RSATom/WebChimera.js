#include "NodeTools.h"

v8::Persistent<v8::Object> thisModule;

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

v8::Local<v8::Function> RequireFunc()
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    Local<Context> context = isolate->GetCurrentContext();

    Local<Object> module = Local<Object>::New(Isolate::GetCurrent(), ::thisModule);
    Local<String > name =
        String::NewFromUtf8(
            isolate,
            "require",
            NewStringType::kInternalized).ToLocalChecked();

    return Local<Function>::Cast(module->Get(context, name).ToLocalChecked());
}

v8::Local<v8::Object> Require(const char* module)
{
    using namespace v8;

    Isolate* isolate = Isolate::GetCurrent();
    Local<Context> context = isolate->GetCurrentContext();
    Local<Object> global = context->Global();

    Local<Value> argv[] =
        { String::NewFromUtf8(isolate, module, NewStringType::kInternalized).ToLocalChecked() };

    return Local<Object>::Cast(RequireFunc()->Call(context, global, 1, argv).ToLocalChecked());
}

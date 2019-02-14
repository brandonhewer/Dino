#include "polymorphic_types/addon.hpp"
#include "polymorphic_types/type_constructor_node.hpp"

namespace {
using namespace Project::Types;

Napi::Value throw_wrong_number_of_product_arguments(Napi::Env &env,
                                                    std::size_t length) {
  Napi::TypeError::New(env, "Wrong number of arguments. Expected 1, received " +
                                std::to_string(length))
      .ThrowAsJavaScriptException();
  return env.Null();
}

Napi::Value throw_invalid_product_argument(Napi::Env &env) {
  Napi::TypeError::New(env, "Expected an integer.")
      .ThrowAsJavaScriptException();
  return env.Null();
}

Napi::Object create_type(Napi::CallbackInfo const &info) {
  return NodeTypeConstructor::create_type(info);
}

Napi::Object create_function_type(Napi::CallbackInfo const &info) {
  return NodeTypeConstructor::function_type(info.Env());
}

Napi::Object create_covariant_type(Napi::CallbackInfo const &info) {
  return NodeTypeConstructor::covariant_type(info.Env());
}

Napi::Object create_contravariant_type(Napi::CallbackInfo const &info) {
  return NodeTypeConstructor::contravariant_type(info.Env());
}

Napi::Value create_product_type(Napi::CallbackInfo const &info) {
  Napi::Env env = info.Env();

  std::size_t length = info.Length();
  if (length != 1)
    return throw_wrong_number_of_product_arguments(env, length);

  if (!info[0].IsNumber())
    return throw_invalid_product_argument(env);

  std::size_t types = info[0].As<Napi::Number>().Uint32Value();
  return NodeTypeConstructor::product_type(env, types);
}

} // namespace

namespace Project {
namespace Types {

Napi::Object init_all(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "create_function_type"),
              Napi::Function::New(env, create_function_type));

  exports.Set(Napi::String::New(env, "create_covariant_type"),
              Napi::Function::New(env, create_covariant_type));

  exports.Set(Napi::String::New(env, "create_contravariant_type"),
              Napi::Function::New(env, create_contravariant_type));

  exports.Set(Napi::String::New(env, "create_product_type"),
              Napi::Function::New(env, create_product_type));

  exports.Set(Napi::String::New(env, "create_type"),
              Napi::Function::New(env, create_type));

  exports.Set(Napi::String::New(env, "TypeConstructor"),
              NodeTypeConstructor::initialize(env));

  return exports;
}

NODE_API_MODULE(PolymorphicTypes, init_all)

} // namespace Types
} // namespace Project

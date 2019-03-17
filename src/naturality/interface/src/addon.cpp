#include "naturality/addon.hpp"
#include "naturality/natural_transformation_node.hpp"

namespace {

using namespace Project::Naturality;

Napi::Value create_natural_transformation(Napi::CallbackInfo const &info) {
  try {
    return NodeNaturalTransformation::create(info);
  } catch (std::runtime_error &err) {
    Napi::TypeError::New(info.Env(), err.what()).ThrowAsJavaScriptException();
    return info.Env().Null();
  }
}

} // namespace

namespace Project {
namespace Naturality {

Napi::Object init_all(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "NaturalTransformation"),
              NodeNaturalTransformation::initialize(env));

  exports.Set(Napi::String::New(env, "createTransformation"),
              Napi::Function::New(env, create_natural_transformation));

  return exports;
}

NODE_API_MODULE(Naturality, init_all)

} // namespace Naturality
} // namespace Project

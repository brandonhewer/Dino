#include "polymorphic_types/atomic_type_reference.hpp"

namespace {

using namespace Project::Types;

NodeAtomicTypeReference &to_atomic_reference(Napi::Object object) {
  return *Napi::ObjectWrap<NodeAtomicTypeReference>::Unwrap(object);
}

Napi::Object escape_object(Napi::EscapableHandleScope &scope,
                           Napi::Object object) {
  return scope.Escape(napi_value(object)).ToObject();
}

Napi::Object
create_atomic_type_reference(Napi::Env &env,
                             Napi::FunctionReference &constructor) {
  Napi::EscapableHandleScope scope(env);
  Napi::Object func = constructor.New({});
  return escape_object(scope, func);
}

} // namespace

namespace Project {
namespace Types {

Napi::FunctionReference NodeAtomicTypeReference::g_constructor;

NodeAtomicTypeReference::NodeAtomicTypeReference(Napi::CallbackInfo const &info)
    : Napi::ObjectWrap<NodeAtomicTypeReference>(info),
      m_context_variance(Variance::INVARIANCE), m_type(nullptr) {}

Napi::Function NodeAtomicTypeReference::initialize(Napi::Env env) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "NodeAtomicTypeReference", {});

  g_constructor = Napi::Persistent(func);
  g_constructor.SuppressDestruct();
  return std::move(func);
}

Napi::Object NodeAtomicTypeReference::create(Napi::Env env,
                                             Variance context_variance,
                                             TypeConstructor::Type &type) {
  auto atomic_object = create_atomic_type_reference(env, g_constructor);
  auto &atomic_type = to_atomic_reference(atomic_object);
  atomic_type.m_context_variance = context_variance;
  atomic_type.m_type = &type;
  return std::move(atomic_object);
}

} // namespace Types
} // namespace Project

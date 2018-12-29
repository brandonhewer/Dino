#include "polymorphic_types/type_constructor_node.hpp"
#include "polymorphic_types/atomic_type_reference.hpp"
#include "polymorphic_types/type_errors.hpp"
#include "polymorphic_types/type_transformers.hpp"

namespace {

using namespace Project::Types;

std::size_t to_unsigned_integer(Napi::Value &value) {
  return value.ToNumber().Uint32Value();
}

NodeTypeConstructor &to_type_constructor(Napi::Object object) {
  return *Napi::ObjectWrap<NodeTypeConstructor>::Unwrap(object);
}

NodeTypeConstructor &to_type_constructor(Napi::Value &value) {
  return to_type_constructor(value.As<Napi::Object>());
}

Napi::Object escape_object(Napi::EscapableHandleScope &scope,
                           Napi::Object object) {
  return scope.Escape(napi_value(object)).ToObject();
}

Napi::Object compose_constructors(NodeTypeConstructor const &lhs,
                                  NodeTypeConstructor const &rhs,
                                  Napi::FunctionReference &constructor) {
  Napi::Object composed_object = constructor.New({});
  auto &composed = to_type_constructor(composed_object);
  composed.set_type_constructor(compose_type_constructors(
      lhs.type_constructor(), rhs.type_constructor()));
  return composed_object;
}

Napi::Value compose_constructors(NodeTypeConstructor const &lhs,
                                 Napi::Value rhs_value, Napi::Env &env,
                                 Napi::FunctionReference &constructor) {
  Napi::EscapableHandleScope scope(env);
  auto const &rhs = to_type_constructor(rhs_value);
  return escape_object(scope, compose_constructors(lhs, rhs, constructor));
}

Napi::Object create_type_constructor(TypeConstructor &&type_constructor,
                                     Napi::Env &env,
                                     Napi::FunctionReference &constructor) {
  Napi::EscapableHandleScope scope(env);
  Napi::Object func = constructor.New({});
  auto &type = to_type_constructor(func);
  type.set_type_constructor(std::move(type_constructor));
  return escape_object(scope, func);
}

Napi::Value throw_wrong_number_of_compose_arguments(Napi::Env &env,
                                                    std::size_t length) {
  Napi::TypeError::New(env, "Wrong number of arguments. Expected 1, received " +
                                std::to_string(length))
      .ThrowAsJavaScriptException();
  return env.Null();
}

Napi::Value throw_invalid_compose_argument(Napi::Env &env) {
  Napi::TypeError::New(env, "Expected a NodeTypeConstructor object.")
      .ThrowAsJavaScriptException();
  return env.Null();
}

Napi::Value throw_composition_error(Napi::Env &env,
                                    CompositionError const &composition) {
  Napi::TypeError::New(env,
                       std::string("Composition failed: ") + composition.what())
      .ThrowAsJavaScriptException();
  return env.Null();
}

Napi::Value throw_wrong_number_of_map_arguments(Napi::Env &env,
                                                std::size_t length) {
  Napi::TypeError::New(env, "Wrong number of arguments. Expected 1, received " +
                                std::to_string(length))
      .ThrowAsJavaScriptException();
  return env.Null();
}

Napi::Value throw_invalid_map_argument(Napi::Env &env) {
  Napi::TypeError::New(env, "Expected a callback.")
      .ThrowAsJavaScriptException();
  return env.Null();
}

Napi::Value throw_invalid_position_argument(Napi::Env &env) {
  Napi::TypeError::New(
      env,
      "Invalid argument passed to type_at, expected only positive integers.")
      .ThrowAsJavaScriptException();
  return env.Null();
}

Napi::Value out_of_bound_position_argument(Napi::Env &env) {
  Napi::TypeError::New(env, "Position passed to type_at is out of bounds.")
      .ThrowAsJavaScriptException();
  return env.Null();
}

TypeConstructor::AtomicType &get_type_at(TypeConstructor::ConstructorType &type,
                                         std::size_t index) {
  if (index >= type.size())
    throw std::out_of_range("");
  return type[index];
}

TypeConstructor::AtomicType &get_type_at(TypeConstructor::ConstructorType &type,
                                         Napi::Value value) {
  if (!value.IsNumber())
    throw std::runtime_error("");
  return get_type_at(type, to_unsigned_integer(value));
}

Variance transform_variance(Variance current, Variance next) {
  if (next == Variance::CONTRAVARIANCE)
    return current == Variance::COVARIANCE ? Variance::CONTRAVARIANCE
                                           : Variance::COVARIANCE;
  return current;
}

std::pair<TypeConstructor::ConstructorType &, Variance>
get_constructor_type_and_variance_at(
    TypeConstructor::ConstructorType &constructor_type,
    Napi::CallbackInfo const &info) {
  Napi::Env env = info.Env();
  std::size_t length = info.Length();
  Variance variance = Variance::COVARIANCE;

  for (auto i = 0u; i < length - 1; ++i) {
    auto &type = get_type_at(constructor_type, info[i]);
    constructor_type = extract_constructor_type(type);
    variance = transform_variance(variance, type.variance);
  }
  return {constructor_type, variance};
}

} // namespace

namespace Project {
namespace Types {

Napi::FunctionReference NodeTypeConstructor::g_constructor;

Napi::Function NodeTypeConstructor::initialize(Napi::Env env) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(
      env, "TypeConstructor",
      {InstanceMethod("compose", &NodeTypeConstructor::compose),
       InstanceMethod("functorise", &NodeTypeConstructor::functorise)});

  g_constructor = Napi::Persistent(func);
  g_constructor.SuppressDestruct();
  return std::move(func);
}

Napi::Object NodeTypeConstructor::function_type(Napi::Env env) {
  return create_type_constructor(create_function_type_constructor(), env,
                                 g_constructor);
}

Napi::Object NodeTypeConstructor::covariant_type(Napi::Env env) {
  return create_type_constructor(create_covariant_type_constructor(), env,
                                 g_constructor);
}

Napi::Object NodeTypeConstructor::contravariant_type(Napi::Env env) {
  return create_type_constructor(create_contravariant_type_constructor(), env,
                                 g_constructor);
}

Napi::Object NodeTypeConstructor::product_type(Napi::Env env,
                                               std::size_t types) {
  return create_type_constructor(create_product_type_constructor(types), env,
                                 g_constructor);
}

NodeTypeConstructor::NodeTypeConstructor(Napi::CallbackInfo const &info)
    : Napi::ObjectWrap<NodeTypeConstructor>(info), m_type_constructor() {}

TypeConstructor const &NodeTypeConstructor::type_constructor() const {
  return m_type_constructor;
}

void NodeTypeConstructor::set_type_constructor(
    TypeConstructor &&type_constructor) {
  m_type_constructor = std::move(type_constructor);
}

Napi::Value NodeTypeConstructor::compose(Napi::CallbackInfo const &info) {
  Napi::Env env = info.Env();

  std::size_t length = info.Length();
  if (length != 1)
    return throw_wrong_number_of_compose_arguments(env, length);

  if (!info[0].IsObject())
    return throw_invalid_compose_argument(env);

  try {
    return compose_constructors(*this, info[0], env, g_constructor);
  } catch (CompositionError const &err) {
    return throw_composition_error(env, err);
  }
}

Napi::Value NodeTypeConstructor::functorise(Napi::CallbackInfo const &info) {
  Napi::Env env = info.Env();

  m_type_constructor.type = {create_covariant_functor_constructor(
      0, std::move(m_type_constructor.type))};

  return info.This();
}

Napi::Value NodeTypeConstructor::type_at(Napi::CallbackInfo const &info) {
  Napi::Env env = info.Env();
  try {
    auto constructor_type =
        get_constructor_type_and_variance_at(m_type_constructor.type, info);
    auto &atomic_type =
        get_type_at(constructor_type.first, info[info.Length() - 1]);
    auto variance =
        transform_variance(constructor_type.second, atomic_type.variance);
    return NodeAtomicTypeReference::create(env, variance, atomic_type.type);
  } catch (std::runtime_error &) {
    return throw_invalid_position_argument(env);
  } catch (std::out_of_range &) {
    return out_of_bound_position_argument(env);
  }
}

} // namespace Types
} // namespace Project

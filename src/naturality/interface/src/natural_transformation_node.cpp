#include "naturality/natural_transformation_node.hpp"
#include "naturality/cospan_composition.hpp"
#include "naturality/cospan_to_string.hpp"
#include "naturality/graph_builder.hpp"
#include "naturality/natural_composition.hpp"
#include "naturality/unify_cospan_with_type.hpp"
#include "polymorphic_types/type_to_string.hpp"
#include "polymorphic_types/unification.hpp"
#include "type_parsers/cospan_parser.hpp"
#include "type_parsers/transformation_parser.hpp"

#include <algorithm>
#include <functional>
#include <numeric>
#include <unordered_map>

#include <iostream>

namespace {

using namespace Project::Naturality;
using namespace Project::Types;

Napi::Value throw_wrong_number_of_graph_arguments(Napi::Env &env,
                                                  std::size_t length) {
  Napi::TypeError::New(
      env, "Wrong number of arguments to 'graph'. Expected 1, received " +
               std::to_string(length))
      .ThrowAsJavaScriptException();
  return env.Null();
}

std::string variance_to_string(Variance variance) {
  switch (variance) {
  case Variance::COVARIANCE:
    return "covariance";
  case Variance::CONTRAVARIANCE:
    return "contravariance";
  case Variance::BIVARIANCE:
    return "bivariance";
  case Variance::INVARIANCE:
    return "invariance";
  }
}

Napi::Value throw_invalid_cospan_structure(Napi::Env env) {
  Napi::TypeError::New(env, "invalid cospan specified")
      .ThrowAsJavaScriptException();
  return env.Null();
}

Napi::Value throw_cospan_parse_error(Napi::Env &&env) {
  Napi::TypeError::New(env, "unable to parse cospan")
      .ThrowAsJavaScriptException();
  return env.Null();
}

Napi::Value throw_failed_to_generate_graph(std::string const &what,
                                           Napi::Env &&env) {
  Napi::TypeError::New(env, "failed to generate graph: " + what)
      .ThrowAsJavaScriptException();
  return env.Null();
}

Napi::Value throw_invalid_arguments_to_compose(Napi::Env env) {
  Napi::TypeError::New(env, "compose expects 1 argument, 0 received")
      .ThrowAsJavaScriptException();
  return env.Null();
}

Napi::Value throw_invalid_argument_type_to_compose(Napi::Env env) {
  Napi::TypeError::New(env,
                       "compose expected natural transformation as argument")
      .ThrowAsJavaScriptException();
  return env.Null();
}

Napi::Value throw_failed_to_compose_types(Napi::Env env) {
  Napi::TypeError::New(env, "failed to compose types")
      .ThrowAsJavaScriptException();
  return env.Null();
}

Napi::Value throw_failed_to_compose_cospans(Napi::Env env) {
  Napi::TypeError::New(env, "failed to compose cospan types")
      .ThrowAsJavaScriptException();
  return env.Null();
}

Napi::Value throw_no_arguments_to_variable(Napi::Env env) {
  Napi::TypeError::New(env, "variable expects 1 argument, 0 received")
      .ThrowAsJavaScriptException();
  return env.Null();
}

Napi::Value throw_invalid_argument_to_variable(Napi::Env env) {
  Napi::TypeError::New(env, "invalid argument passed to variable")
      .ThrowAsJavaScriptException();
  return env.Null();
}

NaturalTransformation create_transformation(Napi::Value const &transform) {
  if (transform.IsString())
    return parse_transformation(transform.ToString().Utf8Value());
  throw std::runtime_error("invalid arguments");
}

NaturalTransformation create_transformation(Napi::Value const &domain_value,
                                            Napi::Value const &codomain_value) {
  if (domain_value.IsString() && codomain_value.IsString())
    return parse_transformation(domain_value.ToString().Utf8Value(),
                                codomain_value.ToString().Utf8Value());
  throw std::runtime_error("invalid arguments");
}

NaturalTransformation create_transformation(Napi::CallbackInfo const &info) {
  if (info.Length() == 0)
    return NaturalTransformation{{{}, {}}, {}};
  else if (info.Length() == 1)
    return create_transformation(info[0]);
  return create_transformation(info[0], info[1]);
}

CospanStructure create_cospan(Napi::Value const &domain,
                              Napi::Value const &codomain) {
  if (domain.IsString() && codomain.IsString())
    return parse_cospan(domain.ToString().Utf8Value(),
                        codomain.ToString().Utf8Value());
  throw std::runtime_error("invalid arguments");
}

CospanStructure create_cospan(Napi::Value const &cospan) {
  if (cospan.IsString())
    return parse_cospan(cospan.ToString().Utf8Value());
  throw std::runtime_error("invalid arguments");
}

CospanStructure create_cospan(Napi::CallbackInfo const &info) {
  if (info.Length() == 1)
    return create_cospan(info[0]);
  return create_cospan(info[0], info[1]);
}

std::size_t get_type(std::string const &symbol,
                     std::vector<std::string> const &symbols) {
  auto it = std::find(symbols.begin(), symbols.end(), symbol);
  if (symbols.end() != it)
    return std::distance(symbols.begin(), it);
  throw std::runtime_error("type " + symbol + " does not exist");
}

std::size_t get_type(std::size_t identifier,
                     std::vector<std::string> const &symbols) {
  if (identifier < symbols.size())
    return identifier;
  throw std::runtime_error("type " + std::to_string(identifier) +
                           " is out of bounds");
}

std::size_t get_type(Napi::Value value,
                     std::vector<std::string> const &symbols) {
  if (value.IsNumber())
    return get_type(value.ToNumber().Uint32Value(), symbols);
  else if (value.IsString())
    return get_type(value.ToString().Utf8Value(), symbols);
  throw std::runtime_error(
      "invalid argument. expected integer or string denoting type variable");
}

NodeNaturalTransformation *get_transformation(Napi::Object object) {
  return Napi::ObjectWrap<NodeNaturalTransformation>::Unwrap(object);
}

NodeNaturalTransformation *get_transformation(Napi::Value value) {
  return get_transformation(value.As<Napi::Object>());
}

std::optional<Unification>
calculate_unification(NaturalTransformation const &left,
                      NaturalTransformation const &right) {
  return calculate_unification(left.domains.back(), right.domains.front(),
                               left.symbols.size(), right.symbols.size(),
                               left.functor_symbols.size(),
                               right.functor_symbols.size());
}

} // namespace

namespace Project {
namespace Naturality {

Napi::FunctionReference NodeNaturalTransformation::g_constructor;

NodeNaturalTransformation::NodeNaturalTransformation(
    Napi::CallbackInfo const &info)
    : Napi::ObjectWrap<NodeNaturalTransformation>(info),
      m_transformation(create_transformation(info)),
      m_type(create_default_cospan(m_transformation.domains[0],
                                   m_transformation.domains[1])),
      m_cospan_value_count(m_transformation.symbols.size(), 1) {}

Napi::Function NodeNaturalTransformation::initialize(Napi::Env env) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(
      env, "NaturalTransformation",
      {InstanceMethod("graph", &NodeNaturalTransformation::graph),
       InstanceMethod("string", &NodeNaturalTransformation::string),
       InstanceMethod("cospanString",
                      &NodeNaturalTransformation::cospan_string),
       InstanceMethod("setCospan", &NodeNaturalTransformation::set_cospan),
       InstanceMethod("compose", &NodeNaturalTransformation::compose),
       InstanceMethod("variable", &NodeNaturalTransformation::variable)});

  g_constructor = Napi::Persistent(func);
  g_constructor.SuppressDestruct();
  return std::move(func);
}

Napi::Object NodeNaturalTransformation::create(Napi::CallbackInfo const &info) {
  if (info.Length() == 0)
    throw std::runtime_error("no arguments passed");
  if (info.Length() == 1)
    return g_constructor.New({info[0]});
  return g_constructor.New({info[0], info[1]});
}

Napi::Value NodeNaturalTransformation::graph(Napi::CallbackInfo const &info) {
  Napi::Env env = info.Env();

  if (info.Length() != 1)
    return throw_wrong_number_of_graph_arguments(env, info.Length());

  try {
    auto const type = get_type(info[0], m_transformation.symbols);
    return generate_graph(m_transformation.domains, m_type,
                          m_cospan_value_count[type], type, env);
  } catch (std::runtime_error &err) {
    return throw_failed_to_generate_graph(err.what(), info.Env());
  }
}

Napi::Value
NodeNaturalTransformation::set_cospan(Napi::CallbackInfo const &info) {
  if (info.Length() == 0)
    throw std::runtime_error("no arguments passed");

  CospanStructure cospan;
  try {
    cospan = create_cospan(info);
  } catch (std::runtime_error &) {
    return throw_cospan_parse_error(info.Env());
  }

  try {
    m_cospan_value_count = unify_cospan_with_type(m_transformation, cospan);
  } catch (std::runtime_error &ex) {
    return throw_invalid_cospan_structure(info.Env());
  }

  m_type = std::move(cospan);
  return info.This();
}

Napi::Value
NodeNaturalTransformation::cospan_string(Napi::CallbackInfo const &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  return Napi::String::New(env, to_string(m_type));
}

Napi::Value NodeNaturalTransformation::string(Napi::CallbackInfo const &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);
  return Napi::String::New(env, to_string(m_transformation));
}

Napi::Value NodeNaturalTransformation::compose(Napi::CallbackInfo const &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  if (info.Length() == 0)
    return throw_invalid_arguments_to_compose(env);

  auto const *right = get_transformation(info[0]);
  auto composite_object = g_constructor.New({});
  auto *composite = get_transformation(composite_object);

  if (!right)
    return throw_invalid_argument_type_to_compose(env);

  auto unification =
      calculate_unification(m_transformation, right->m_transformation);

  if (!unification)
    return throw_failed_to_compose_types(env);

  composite->m_transformation = compose_transformations(
      m_transformation, right->m_transformation, *unification);

  auto composition = compose_cospans(
      m_type, right->m_type, m_transformation, right->m_transformation,
      *unification, composite->m_transformation.symbols.size());
  composite->m_type = std::move(composition.cospan);
  composite->m_cospan_value_count = std::move(composition.value_count);
  return composite_object;
}

Napi::Value
NodeNaturalTransformation::variable(Napi::CallbackInfo const &info) {
  Napi::Env env = info.Env();
  Napi::EscapableHandleScope scope(env);

  if (info.Length() == 0)
    return throw_no_arguments_to_variable(env);

  auto const value = info[0];
  if (!value.IsNumber())
    return throw_invalid_argument_to_variable(env);

  auto const &symbol = m_transformation.symbols[value.ToNumber().Uint32Value()];
  return Napi::String::New(env, symbol);
}

} // namespace Naturality
} // namespace Project

#include "naturality/natural_transformation_node.hpp"
#include "naturality/natural_transformers.hpp"
#include "polymorphic_types/type_transformers.hpp"
#include "type_parsers/cospan_parser.hpp"
#include "type_parsers/transformation_parser.hpp"

#include <algorithm>
#include <functional>
#include <numeric>
#include <unordered_map>

namespace {

using namespace Project::Naturality;
using namespace Project::Types;

struct TypeNode {
  std::size_t type;
  bool is_domain;
};

struct GraphData {
  std::vector<std::vector<Napi::Object>> nodes;
  std::vector<Napi::Object> transitions;
  std::vector<Napi::Object> incoming_edges;
  std::vector<Napi::Object> outgoing_edges;
  std::vector<Napi::Object> invisible_edges;
  std::vector<std::size_t> transition_ids;
  std::size_t node_count;
};

Napi::Value escape_value(Napi::EscapableHandleScope &scope, Napi::Value value) {
  return scope.Escape(napi_value(value));
}

Napi::Object create_type_node(std::size_t identifier, TypeNode const &node,
                              Variance variance, Napi::Env &env) {
  Napi::Object object = Napi::Object::New(env);
  object.Set("id", identifier);
  object.Set("type", node.type);
  object.Set("variance", static_cast<std::size_t>(variance));
  object.Set("count", 0);
  return std::move(object);
}

Napi::Object create_transition_node(std::size_t identifier, Napi::Env &env) {
  Napi::Object object = Napi::Object::New(env);
  object.Set("id", identifier);
  return std::move(object);
}

Napi::Object create_edge(std::size_t source, std::size_t target,
                         std::size_t distance, Napi::Env &env) {
  Napi::Object object = Napi::Object::New(env);
  object.Set("source", source);
  object.Set("target", target);
  object.Set("distance", distance);
  return std::move(object);
}

Variance invert_variance(Variance variance) {
  switch (variance) {
  case Variance::CONTRAVARIANCE:
    return Variance::COVARIANCE;
  case Variance::COVARIANCE:
    return Variance::CONTRAVARIANCE;
  default:
    return variance;
  }
}

Variance calculate_variance(Variance variance, Variance environment_variance) {
  switch (environment_variance) {
  case Variance::CONTRAVARIANCE:
    return invert_variance(variance);
  default:
    return variance;
  }
}

bool is_edge_source(TypeNode const &type_node, Variance variance) {
  return (type_node.is_domain && Variance::COVARIANCE == variance) ||
         (!type_node.is_domain && Variance::CONTRAVARIANCE == variance);
}

void create_edge(GraphData &graph, std::size_t node, TypeNode const &type_node,
                 Variance variance, std::size_t transition, Napi::Env &env) {
  if (is_edge_source(type_node, variance))
    graph.incoming_edges.emplace_back(create_edge(node, transition, 2, env));
  else
    graph.outgoing_edges.emplace_back(create_edge(transition, node, 2, env));
}

void generate_graph_part(GraphData &, std::size_t, TypeNode const &, Variance,
                         Napi::Env &, TypeConstructor const &,
                         CospanMorphism const &);

void generate_graph_part(GraphData &, std::size_t, TypeNode const &, Variance,
                         Napi::Env &, FunctorTypeConstructor const &,
                         CospanMorphism const &);

struct GenerateGraphPart {

  void operator()(GraphData &graph, std::size_t part, TypeNode const &type_node,
                  Variance variance, Napi::Env &env,
                  TypeConstructor const &constructor,
                  CospanMorphism const &morphism) const {
    generate_graph_part(graph, part, type_node, variance, env, constructor,
                        morphism);
  }

  void operator()(GraphData &graph, std::size_t part, TypeNode const &type_node,
                  Variance variance, Napi::Env &env, std::size_t type,
                  std::size_t transition) const {
    if (type == type_node.type) {
      auto const transition_id =
          std::distance(graph.transition_ids.begin(),
                        std::find(graph.transition_ids.begin(),
                                  graph.transition_ids.end(), transition));
      create_edge(graph, graph.node_count, type_node, variance, transition_id,
                  env);
      auto &nodes = graph.nodes[part];
      nodes.emplace_back(
          create_type_node(graph.node_count, type_node, variance, env));
      ++graph.node_count;
    }
  }

  void operator()(GraphData &graph, std::size_t part, TypeNode const &type_node,
                  Variance variance, Napi::Env &env,
                  FunctorTypeConstructor const &functor,
                  CospanMorphism const &morphism) const {
    generate_graph_part(graph, part, type_node, variance, env, functor,
                        morphism);
  }

  template <typename T>
  void operator()(GraphData &, std::size_t, TypeNode const &, Variance,
                  Napi::Env &, FunctorTypeConstructor const &,
                  T const &) const {
    throw std::runtime_error("cospan type does not match polymorphic type");
  }

  template <typename T>
  void operator()(GraphData &, std::size_t, TypeNode const &, Variance,
                  Napi::Env &, TypeConstructor const &, T const &) const {
    throw std::runtime_error("cospan type does not match polymorphic type");
  }

  template <typename T>
  void operator()(GraphData &, std::size_t, TypeNode const &, Variance,
                  Napi::Env &, T const &, CospanMorphism const &) const {
    throw std::runtime_error("cospan type does not match polymorphic type");
  }

  template <typename T, typename U>
  void operator()(GraphData &, std::size_t, TypeNode const &, Variance,
                  Napi::Env &, T const &, U const &) const {}
} _generate_graph_part;

template <typename F>
void generate_graph_part(F const &create_graph_part, Variance variance,
                         TypeConstructor::AtomicType const &type,
                         CospanMorphism::MappedType const &cospan_type) {
  if (type.variance != cospan_type.variance)
    throw std::runtime_error("cospan type does not match polymorphic type");

  auto const create_part =
      std::bind(create_graph_part, calculate_variance(type.variance, variance),
                std::placeholders::_1, std::placeholders::_2);
  std::visit(create_part, type.type, cospan_type.type);
}

void generate_graph_part(GraphData &graph, std::size_t part,
                         TypeNode const &type_node, Variance variance,
                         Napi::Env &env,
                         TypeConstructor::ConstructorType const &types,
                         CospanMorphism const &morphism) {
  if (morphism.map.size() != types.size())
    throw std::runtime_error("cospan type does not match polymorphic type");

  auto const create_graph_part =
      std::bind(_generate_graph_part, std::ref(graph), part,
                std::cref(type_node), std::placeholders::_1, std::ref(env),
                std::placeholders::_2, std::placeholders::_3);

  for (auto i = 0u; i < morphism.map.size(); ++i)
    generate_graph_part(create_graph_part, variance, types[i], morphism.map[i]);
}

void generate_graph_part(GraphData &graph, std::size_t part,
                         TypeNode const &type_node, Variance variance,
                         Napi::Env &env, FunctorTypeConstructor const &functor,
                         CospanMorphism const &morphism) {
  return generate_graph_part(graph, part, type_node, variance, env,
                             functor.type, morphism);
}

void generate_graph_part(GraphData &graph, std::size_t part,
                         TypeNode const &type_node, Variance variance,
                         Napi::Env &env, TypeConstructor const &constructor,
                         CospanMorphism const &morphism) {
  return generate_graph_part(graph, part, type_node, variance, env,
                             constructor.type, morphism);
}

std::vector<Napi::Object> generate_invisible_edges(std::size_t transitions,
                                                   Napi::Env &env) {
  std::vector<Napi::Object> edges;
  for (auto i = 0u; i < transitions - 1; ++i)
    edges.emplace_back(create_edge(i, i + 1, 1, env));
  return std::move(edges);
}

std::vector<Napi::Object>
generate_transitions(std::vector<std::size_t> const &transitions,
                     Napi::Env &env) {
  std::vector<Napi::Object> nodes;
  nodes.reserve(transitions.size());
  for (auto &&transition : transitions)
    nodes.emplace_back(create_transition_node(transition, env));
  return std::move(nodes);
}

Napi::Value create_napi_array(std::vector<Napi::Object> const &objects,
                              Napi::Env &env) {
  Napi::Array array = Napi::Array::New(env, objects.size());
  for (auto i = 0u; i < objects.size(); ++i)
    array[i] = objects[i];
  return std::move(array);
}

Napi::Value
create_napi_array(std::vector<std::vector<Napi::Object>> const &objects,
                  Napi::Env &env) {
  Napi::Array array = Napi::Array::New(env, objects.size());
  for (auto i = 0u; i < objects.size(); ++i)
    array[i] = create_napi_array(objects[i], env);
  return std::move(array);
}

Napi::Value
create_edges_object(std::vector<Napi::Object> const &incoming_edges,
                    std::vector<Napi::Object> const &outgoing_edges,
                    std::vector<Napi::Object> const &invisible_edges,
                    Napi::Env &env) {
  Napi::Object edges = Napi::Object::New(env);
  edges.Set("incoming", create_napi_array(incoming_edges, env));
  edges.Set("outgoing", create_napi_array(outgoing_edges, env));
  edges.Set("invisible", create_napi_array(invisible_edges, env));
  return std::move(edges);
}

Napi::Value create_graph(GraphData &data, Napi::Env &env,
                         Napi::EscapableHandleScope &scope) {
  Napi::Object graph = Napi::Object::New(env);
  graph.Set("nodes", create_napi_array(data.nodes, env));
  graph.Set("transitions", create_napi_array(data.transitions, env));
  graph.Set("edges",
            create_edges_object(data.incoming_edges, data.outgoing_edges,
                                data.invisible_edges, env));
  return escape_value(scope, graph);
}

Napi::Value generate_graph(TypeConstructor const &domain,
                           TypeConstructor const &codomain,
                           CospanStructure const &cospan, std::size_t type,
                           Napi::Env &env) {
  GraphData graph;
  graph.nodes = {{}, {}};
  graph.node_count = 0;

  Napi::EscapableHandleScope scope(env);
  graph.transition_ids = extract_cospan_values(cospan);
  graph.transitions = generate_transitions(graph.transition_ids, env);
  graph.node_count = graph.transitions.size();
  graph.invisible_edges = generate_invisible_edges(graph.node_count, env);
  generate_graph_part(graph, 0, {type, true}, Variance::COVARIANCE, env, domain,
                      cospan.domain);
  generate_graph_part(graph, 1, {type, false}, Variance::COVARIANCE, env,
                      codomain, cospan.codomain);
  return create_graph(graph, env, scope);
}

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

struct CompatibilityErrorToString {

  std::string operator()(std::vector<std::string> const &symbols,
                         StructureError const &error) const {
    return "invalid structure: " + to_string(error.cospan_type) +
           " is incompatible with " + to_string(error.type, symbols);
  }

  std::string operator()(std::vector<std::string> const &symbols,
                         VarianceError const &error) const {
    return "invalid variance: " +
           variance_to_string(error.cospan_type.variance) + " of " +
           to_string(error.cospan_type.type) + " is incompatible with the " +
           variance_to_string(error.type.variance) + " of " +
           to_string(error.type.type, symbols);
  }

} _compatibility_error_to_string;

std::string
compatibility_error_to_string(CompatibilityError const &error,
                              std::vector<std::string> const &symbols) {
  return std::visit(std::bind(_compatibility_error_to_string,
                              std::cref(symbols), std::placeholders::_1),
                    error);
}

Napi::Value
throw_invalid_cospan_structure(Napi::Env env, CompatibilityError const &error,
                               std::vector<std::string> const &symbols) {
  Napi::TypeError::New(env, "invalid cospan specified: " +
                                compatibility_error_to_string(error, symbols))
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
  if (info.Length() == 1)
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

} // namespace

namespace Project {
namespace Naturality {

Napi::FunctionReference NodeNaturalTransformation::g_constructor;

NodeNaturalTransformation::NodeNaturalTransformation(
    Napi::CallbackInfo const &info)
    : Napi::ObjectWrap<NodeNaturalTransformation>(info),
      m_transformation(create_transformation(info)),
      m_type(create_default_cospan(m_transformation.domain,
                                   m_transformation.codomain)) {}

Napi::Function NodeNaturalTransformation::initialize(Napi::Env env) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(
      env, "NaturalTransformation",
      {InstanceMethod("graph", &NodeNaturalTransformation::graph),
       InstanceMethod("string", &NodeNaturalTransformation::string),
       InstanceMethod("cospan_string",
                      &NodeNaturalTransformation::cospan_string),
       InstanceMethod("set_cospan", &NodeNaturalTransformation::set_cospan)});

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

  auto const type = get_type(info[0], m_transformation.symbols);
  try {
    return generate_graph(m_transformation.domain, m_transformation.codomain,
                          m_type, type, env);
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

  if (auto error = is_incompatible(m_transformation, cospan))
    return throw_invalid_cospan_structure(info.Env(), *error,
                                          m_transformation.symbols);
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

} // namespace Naturality
} // namespace Project

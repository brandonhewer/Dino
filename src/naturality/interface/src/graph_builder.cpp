#include "naturality/graph_builder.hpp"

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

bool is_edge_source(TypeNode const &type_node, Variance variance) {
  return (type_node.is_domain && Variance::COVARIANCE == variance) ||
         (!type_node.is_domain && Variance::CONTRAVARIANCE == variance);
}

Napi::Object create_type_node(std::size_t identifier, TypeNode const &node,
                              Variance variance, Napi::Env &env) {
  Napi::Object object = Napi::Object::New(env);
  object.Set("id", identifier);
  object.Set("type", node.type);
  object.Set("variance", static_cast<std::size_t>(variance));
  object.Set("count", is_edge_source(node, variance) ? 1 : 0);
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

std::size_t find_transition_id(GraphData &graph, std::size_t transition) {
  return std::distance(graph.transition_ids.begin(),
                       std::find(graph.transition_ids.begin(),
                                 graph.transition_ids.end(), transition));
}

void add_graph_node(GraphData &graph, std::size_t part,
                    TypeNode const &type_node, Variance variance,
                    Napi::Env &env) {
  auto &nodes = graph.nodes[part];
  nodes.emplace_back(
      create_type_node(graph.node_count, type_node, variance, env));
  ++graph.node_count;
}

void add_graph_edge(GraphData &graph, TypeNode const &type_node,
                    Variance variance, Napi::Env &env, std::size_t transition) {
  create_edge(graph, graph.node_count, type_node, variance,
              find_transition_id(graph, transition), env);
}

void create_graph_part(GraphData &graph, std::size_t part,
                       TypeNode const &type_node, Variance variance,
                       Napi::Env &env, std::size_t type,
                       std::size_t transition) {
  if (type == type_node.type) {
    add_graph_edge(graph, type_node, variance, env, transition);
    add_graph_node(graph, part, type_node, variance, env);
  }
}

void create_graph_part(GraphData &graph, std::size_t part,
                       TypeNode const &type_node, Variance variance,
                       Napi::Env &env, std::size_t type,
                       std::pair<std::size_t, std::size_t> const &transitions) {
  if (type == type_node.type) {
    add_graph_edge(graph, {type, false}, variance, env, transitions.first);
    add_graph_edge(graph, {type, true}, variance, env, transitions.second);
    add_graph_node(graph, part, type_node, variance, env);
  }
}

struct GenerateGraphPart {

  void operator()(GraphData &graph, std::size_t part, TypeNode const &type_node,
                  Variance variance, Napi::Env &env, std::size_t type,
                  std::size_t transition) const {
    create_graph_part(graph, part, type_node, variance, env, type, transition);
  }

  void operator()(GraphData &graph, std::size_t part, TypeNode const &type_node,
                  Variance variance, Napi::Env &env, std::size_t type,
                  std::pair<std::size_t, std::size_t> const &transitions) {
    create_graph_part(graph, part, type_node, variance, env, type, transitions);
  }

  void operator()(GraphData &graph, std::size_t part, TypeNode const &type_node,
                  Variance variance, Napi::Env &env,
                  TypeConstructor const &constructor,
                  CospanMorphism const &morphism) const {
    generate_graph_part(graph, part, type_node, variance, env, constructor,
                        morphism);
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

void generate_graph_parts(GraphData &graph,
                          std::vector<TypeConstructor> const &domains,
                          CospanStructure const &cospan, std::size_t type,
                          Napi::Env &env) {
  if (domains.empty())
    return;

  generate_graph_part(graph, 0, {type, true}, Variance::COVARIANCE, env,
                      domains[0], cospan.domains[0]);

  for (auto i = 1u; i < domains.size(); ++i)
    generate_graph_part(graph, i, {type, false}, Variance::COVARIANCE, env,
                        domains[i], cospan.domains[i]);
}

} // namespace

namespace Project {
namespace Naturality {

Napi::Value generate_graph(std::vector<TypeConstructor> const &domains,
                           CospanStructure const &cospan, std::size_t type,
                           Napi::Env &env) {
  GraphData graph;
  graph.nodes = std::vector<std::vector<Napi::Object>>(
      domains.size(), std::vector<Napi::Object>());
  graph.node_count = 0;

  Napi::EscapableHandleScope scope(env);
  graph.transition_ids = extract_cospan_values(cospan);
  graph.transitions = generate_transitions(graph.transition_ids, env);
  graph.node_count = graph.transitions.size();
  graph.invisible_edges = generate_invisible_edges(graph.node_count, env);

  generate_graph_parts(graph, domains, cospan, type, env);
  return create_graph(graph, env, scope);
}

} // namespace Naturality
} // namespace Project

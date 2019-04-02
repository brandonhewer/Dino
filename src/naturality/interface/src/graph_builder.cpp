#include "naturality/graph_builder.hpp"
#include "naturality/cospan_equality.hpp"
#include "polymorphic_types/type_equality.hpp"

#include <naturality/cospan_to_string.hpp>

namespace {

using namespace Project::Naturality;
using namespace Project::Types;

struct TypeNode {
  std::size_t type;
  bool is_domain;
};

struct GraphData {
  std::vector<std::vector<Napi::Object>> nodes;
  std::vector<std::vector<Napi::Object>> transitions;
  std::vector<Napi::Object> incoming_edges;
  std::vector<Napi::Object> outgoing_edges;
  std::vector<Napi::Object> invisible_edges;
  std::size_t transition_ids;
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

Napi::Object create_type_node(std::size_t identifier, std::size_t type,
                              Variance variance, Napi::Env &env) {
  Napi::Object object = Napi::Object::New(env);
  object.Set("id", identifier);
  object.Set("type", type);
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
                         double distance, Napi::Env &env) {
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
    graph.incoming_edges.emplace_back(create_edge(node, transition, 2.0, env));
  else
    graph.outgoing_edges.emplace_back(create_edge(transition, node, 2.0, env));
}

void generate_graph_part(GraphData &, std::size_t, TypeNode const &, Variance,
                         Napi::Env &, TypeConstructor const &,
                         CospanMorphism const &);

void generate_graph_part(GraphData &, std::size_t, TypeNode const &, Variance,
                         Napi::Env &, FunctorTypeConstructor const &,
                         CospanMorphism const &);

void add_graph_node(GraphData &graph, std::size_t part,
                    TypeNode const &type_node, Variance variance,
                    Napi::Env &env) {
  auto &nodes = graph.nodes[part];
  nodes.emplace_back(
      create_type_node(graph.node_count, type_node, variance, env));
  ++graph.node_count;
}

void add_graph_node(GraphData &graph, std::size_t part, std::size_t type,
                    Variance variance, Napi::Env &env) {
  auto &nodes = graph.nodes[part];
  nodes.emplace_back(create_type_node(graph.node_count, type, variance, env));
  ++graph.node_count;
}

void add_graph_edge(GraphData &graph, TypeNode const &type_node,
                    Variance variance, Napi::Env &env, std::size_t transition) {
  create_edge(graph, graph.node_count, type_node, variance, transition, env);
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
                       CospanMorphism::PairType const &transitions) {
  if (type == type_node.type) {
    add_graph_edge(graph, {type, false}, variance, env, transitions.first);
    add_graph_edge(graph, {type, true}, variance, env, transitions.second);
    add_graph_node(graph, part, type_node.type, variance, env);
  }
}

template <typename T>
void generate_graph_part_with(GraphData &, std::size_t, TypeNode const &,
                              Variance, Napi::Env &,
                              TypeConstructor::Type const &, T const &);

template <typename T>
void generate_graph_part_with(GraphData &, std::size_t, TypeNode const &,
                              Variance, Napi::Env &, T const &,
                              CospanMorphism::Type const &);

struct GenerateGraphPart {

  void operator()(GraphData &graph, std::size_t part, TypeNode const &type_node,
                  Variance variance, Napi::Env &env, std::size_t type,
                  std::size_t transition) const {
    create_graph_part(graph, part, type_node, variance, env, type, transition);
  }

  void operator()(GraphData &graph, std::size_t part, TypeNode const &type_node,
                  Variance variance, Napi::Env &env, std::size_t type,
                  CospanMorphism::PairType const &transitions) const {
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
  void operator()(GraphData &graph, std::size_t part, TypeNode const &type_node,
                  Variance variance, Napi::Env &env,
                  FunctorTypeConstructor const &functor,
                  T const &cospan_value) const {
    if (functor.type.size() == 1)
      generate_graph_part_with(graph, part, type_node, variance, env,
                               functor.type[0].type, cospan_value);
    else
      throw std::runtime_error(
          "cospan type does not match polymorphic type: functor found");
  }

  template <typename T>
  void operator()(GraphData &graph, std::size_t part, TypeNode const &type_node,
                  Variance variance, Napi::Env &env,
                  TypeConstructor const &type, T const &cospan_value) const {
    if (type.type.size() == 1)
      generate_graph_part_with(graph, part, type_node, variance, env,
                               type.type[0].type, cospan_value);
    else
      throw std::runtime_error(
          "cospan type does not match polymorphic type: constructor found");
  }

  template <typename T>
  void operator()(GraphData &graph, std::size_t part, TypeNode const &type_node,
                  Variance variance, Napi::Env &env, T const &type,
                  CospanMorphism const &morphism) const {
    if (morphism.map.size() == 1)
      generate_graph_part_with(graph, part, type_node, variance, env, type,
                               morphism.map[0]);
    else
      throw std::runtime_error(
          "cospan type does not match polymorphic type: morphism found");
  }

  template <typename T, typename U>
  void operator()(GraphData &, std::size_t, TypeNode const &, Variance,
                  Napi::Env &, T const &, U const &) const {
    throw std::runtime_error(
        "cospan type does not match polymorphic type: unknown");
  }
} _generate_graph_part;

template <typename T>
void generate_graph_part_with(GraphData &graph, std::size_t part,
                              TypeNode const &type_node, Variance variance,
                              Napi::Env &env, TypeConstructor::Type const &type,
                              T const &cospan_value) {
  std::visit(std::bind(_generate_graph_part, std::ref(graph), part,
                       std::cref(type_node), variance, std::ref(env),
                       std::placeholders::_1, std::cref(cospan_value)),
             type);
}

template <typename T>
void generate_graph_part_with(GraphData &graph, std::size_t part,
                              TypeNode const &type_node, Variance variance,
                              Napi::Env &env, T const &type,
                              CospanMorphism::Type const &cospan_value) {
  std::visit(std::bind(_generate_graph_part, std::ref(graph), part,
                       std::cref(type_node), variance, std::ref(env),
                       std::cref(type), std::placeholders::_1),
             cospan_value);
}

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
  auto const &nested_morphism = get_nested(morphism);

  if (nested_morphism.map.size() == functor.type.size())
    generate_graph_part(graph, part, type_node, variance, env, functor.type,
                        nested_morphism);
  else if (nested_morphism.map.size() == 1)
    generate_graph_part_with(graph, part, type_node, variance, env, functor,
                             nested_morphism.map[0]);
  else if (functor.type.size() == 1)
    generate_graph_part_with(graph, part, type_node, variance, env,
                             functor.type[0].type, nested_morphism);
  else
    throw std::runtime_error("cospan type does not match polymorphic type");
}

void generate_graph_part(GraphData &graph, std::size_t part,
                         TypeNode const &type_node, Variance variance,
                         Napi::Env &env, TypeConstructor const &constructor,
                         CospanMorphism const &morphism) {
  auto const &nested_constructor = get_nested(constructor);
  auto const &nested_morphism = get_nested(morphism);

  if (nested_morphism.map.size() == nested_constructor.type.size())
    generate_graph_part(graph, part, type_node, variance, env,
                        nested_constructor.type, nested_morphism);
  else if (nested_morphism.map.size() == 1)
    generate_graph_part_with(graph, part, type_node, variance, env,
                             nested_constructor, nested_morphism.map[0]);
  else if (nested_constructor.type.size() == 1)
    generate_graph_part_with(graph, part, type_node, variance, env,
                             nested_constructor.type[0].type, nested_morphism);
  else
    throw std::runtime_error("cospan type does not match polymorphic type");
}

void add_invisible_edges(std::vector<Napi::Object> &edges, std::size_t from,
                         std::size_t to, Napi::Env &env) {
  if (0 == to || from == to)
    return;

  edges.reserve(edges.size() + (to - from));
  for (auto i = from; i < to - 1; ++i)
    edges.emplace_back(create_edge(i, i + 1, 0.5, env));
}

void add_invisible_edges(std::vector<Napi::Object> &edges,
                         std::vector<Napi::Object> const &nodes,
                         std::size_t &start, Napi::Env &env) {
  for (auto i = 0u; i < nodes.size() - 1; ++i) {
    edges.emplace_back(create_edge(start, start + 1, 1.0, env));
    start += 1;
  }
  start += 1;
}

void add_invisible_edges(std::vector<Napi::Object> &edges,
                         std::vector<std::vector<Napi::Object>> const &nodes,
                         std::size_t start, Napi::Env &env) {
  for (auto &&node_group : nodes) {
    if (node_group.size() > 0)
      add_invisible_edges(edges, node_group, start, env);
  }
}

std::vector<Napi::Object> generate_invisible_edges(
    std::size_t transitions,
    std::vector<std::pair<std::size_t, std::size_t>> const &shared_count,
    Napi::Env &env) {
  std::vector<Napi::Object> edges;
  auto const edge_sets = shared_count.size() - 1;
  std::size_t current = 0;

  for (auto i = 0u; i < edge_sets; ++i) {
    auto const shared = shared_count[i].second;
    auto const maximum = shared + current;

    if (transitions > maximum)
      add_invisible_edges(edges, current, maximum, env);
    else {
      add_invisible_edges(edges, current, transitions, env);
      return std::move(edges);
    }

    current = maximum;
  }
  return std::move(edges);
}

std::vector<Napi::Object> generate_invisible_edges(
    std::vector<std::pair<std::size_t, std::size_t>> const &transitions,
    Napi::Env &env) {
  std::vector<Napi::Object> edges;
  edges.reserve(transitions.back().second - transitions.size());
  for (auto &&transition : transitions) {
    for (auto i = transition.first; i < transition.second - 1; ++i)
      edges.emplace_back(create_edge(i, i + 1, 0.5, env));
  }
  return std::move(edges);
}

std::vector<std::pair<std::size_t, std::size_t>> group_transitions(
    std::size_t transitions,
    std::vector<std::pair<std::size_t, std::size_t>> const &shared_count) {
  std::vector<std::pair<std::size_t, std::size_t>> grouped;
  grouped.reserve(shared_count.size() - 1);

  std::size_t current_count = 0u;
  for (auto i = 0u; i < shared_count.size() - 1; ++i) {
    auto const &count = shared_count[i];
    grouped.emplace_back(current_count, current_count + count.second);
    current_count += count.second;
  }
  return std::move(grouped);
}

std::vector<Napi::Object> generate_transitions(std::size_t transitions,
                                               Napi::Env &env) {
  std::vector<Napi::Object> nodes;
  nodes.reserve(transitions);
  for (auto transition = 0u; transition < transitions; ++transition)
    nodes.emplace_back(create_transition_node(transition, env));
  return std::move(nodes);
}

std::vector<Napi::Object>
generate_transitions(std::pair<std::size_t, std::size_t> const &transitions,
                     Napi::Env &env) {
  std::vector<Napi::Object> nodes;
  nodes.reserve(transitions.second - transitions.first);
  for (auto i = transitions.first; i < transitions.second; ++i)
    nodes.emplace_back(create_transition_node(i, env));
  return std::move(nodes);
}

std::vector<std::vector<Napi::Object>> generate_transitions(
    std::vector<std::pair<std::size_t, std::size_t>> const &transitions,
    Napi::Env &env) {
  std::vector<std::vector<Napi::Object>> nodes;
  nodes.reserve(transitions.size());
  for (auto &&transition_group : transitions)
    nodes.emplace_back(generate_transitions(transition_group, env));
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
                           CospanStructure const &cospan,
                           std::size_t transitions, std::size_t type,
                           Napi::Env &env) {
  GraphData graph;
  graph.nodes = std::vector<std::vector<Napi::Object>>(
      domains.size(), std::vector<Napi::Object>());

  Napi::EscapableHandleScope scope(env);
  graph.node_count = transitions;

  auto const &grouped_transitions =
      group_transitions(transitions, cospan.shared_counts);
  graph.invisible_edges = generate_invisible_edges(grouped_transitions, env);
  graph.transitions = generate_transitions(grouped_transitions, env);

  generate_graph_parts(graph, domains, cospan, type, env);
  add_invisible_edges(graph.invisible_edges, graph.nodes, transitions, env);

  return create_graph(graph, env, scope);
}

} // namespace Naturality
} // namespace Project

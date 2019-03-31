#include "naturality/unify_cospan_with_type.hpp"
#include "naturality/cospan_equality.hpp"

#include "polymorphic_types/type_equality.hpp"

#include <numeric>
#include <optional>

namespace {

using namespace Project::Naturality;
using namespace Project::Types;

using CospanUnifiers = std::vector<std::vector<std::optional<std::size_t>>>;

void unify_cospan_with_type(CospanUnifiers &, std::vector<std::size_t> &,
                            TypeConstructor::Type const &,
                            CospanMorphism::Type &);

template <typename T>
void unify_cospan_with_type_with(CospanUnifiers &, std::vector<std::size_t> &,
                                 T const &, CospanMorphism::Type &);

template <typename T>
void unify_cospan_with_type_with(CospanUnifiers &, std::vector<std::size_t> &,
                                 TypeConstructor::Type const &, T &);

void unify_constructor_with_morphism(
    CospanUnifiers &unified, std::vector<std::size_t> &counts,
    TypeConstructor::ConstructorType const &constructor,
    std::vector<CospanMorphism::MappedType> &morphism) {
  for (auto i = 0u; i < morphism.size(); ++i) {
    auto &cospan_type = morphism[i];
    auto const &type = constructor[i];

    if (cospan_type.variance != type.variance)
      throw std::runtime_error("variance error");
    unify_cospan_with_type(unified, counts, type.type, cospan_type.type);
  }
}

void unify_constructor_with_morphism(CospanUnifiers &unified,
                                     std::vector<std::size_t> &counts,
                                     TypeConstructor const &constructor,
                                     CospanMorphism &morphism) {
  auto const &nested_constructor = get_nested(constructor);
  auto &nested_morphism = get_nested(morphism);
  auto const constructor_size = nested_constructor.type.size();
  auto const morphism_size = nested_morphism.map.size();

  if (constructor_size == morphism_size)
    unify_constructor_with_morphism(unified, counts, nested_constructor.type,
                                    nested_morphism.map);
  else if (constructor_size == 1)
    unify_cospan_with_type_with(
        unified, counts, nested_constructor.type[0].type, nested_morphism);
  else if (morphism_size == 1)
    unify_cospan_with_type_with(unified, counts, nested_constructor,
                                nested_morphism.map[0].type);
  else
    throw std::runtime_error("structure error (constructor, morphism)");
}

void unify_functor_with_morphism(CospanUnifiers &unified,
                                 std::vector<std::size_t> &counts,
                                 FunctorTypeConstructor const &functor,
                                 CospanMorphism &morphism) {
  auto &nested_morphism = get_nested(morphism);
  auto const functor_size = functor.type.size();
  auto const morphism_size = nested_morphism.map.size();

  if (functor_size == morphism_size)
    unify_constructor_with_morphism(unified, counts, functor.type,
                                    nested_morphism.map);
  else if (functor_size == 1)
    unify_cospan_with_type_with(unified, counts, functor.type[0].type,
                                nested_morphism);
  else if (morphism_size == 1)
    unify_cospan_with_type_with(unified, counts, functor,
                                nested_morphism.map[0].type);
  else
    throw std::runtime_error("structure error (functor, morphism)");
}

void unify_identifier_with(CospanUnifiers &unified,
                           std::vector<std::size_t> &counts,
                           std::size_t identifier, std::size_t &cospan_value) {
  auto &identifier_unified = unified[identifier];
  auto &count = counts[identifier];

  if (auto const value = identifier_unified[cospan_value])
    cospan_value = *value;
  else {
    identifier_unified[cospan_value] = count;
    cospan_value = count++;
  }
}

struct UnifyCospanWithType {

  void operator()(CospanUnifiers &unified, std::vector<std::size_t> &counts,
                  std::size_t identifier, std::size_t &cospan_value) const {
    unify_identifier_with(unified, counts, identifier, cospan_value);
  }

  void operator()(CospanUnifiers &unified, std::vector<std::size_t> &counts,
                  TypeConstructor const &constructor,
                  CospanMorphism &morphism) const {
    unify_constructor_with_morphism(unified, counts, constructor, morphism);
  }

  void operator()(CospanUnifiers &unified, std::vector<std::size_t> &counts,
                  FunctorTypeConstructor const &functor,
                  CospanMorphism &morphism) const {
    unify_functor_with_morphism(unified, counts, functor, morphism);
  }

  template <typename T>
  void operator()(CospanUnifiers &unified, std::vector<std::size_t> &counts,
                  T const &type, CospanMorphism &morphism) const {
    auto const &nested = get_nested(morphism);
    if (nested.map.size() != 1)
      throw std::runtime_error("structure error (type, morphism)");
    unify_cospan_with_type_with(unified, counts, type, nested.map[0]);
  }

  template <typename T>
  void operator()(CospanUnifiers &unified, std::vector<std::size_t> &counts,
                  TypeConstructor const &constructor, T &cospan_type) const {
    auto const &nested = get_nested(constructor);
    if (nested.type.size() != 1)
      throw std::runtime_error("structure error (constructor, type)");
    unify_cospan_with_type_with(unified, counts, nested.type[0].type,
                                cospan_type);
  }

  template <typename T>
  void operator()(CospanUnifiers &unified, std::vector<std::size_t> &counts,
                  FunctorTypeConstructor const &functor, T &cospan_type) const {
    if (functor.type.size() != 1)
      throw std::runtime_error("structure error (functor, type)");
    unify_cospan_with_type_with(unified, counts, functor.type[0].type,
                                cospan_type);
  }

  template <typename T, typename U>
  void operator()(CospanUnifiers &, std::vector<std::size_t> &, T const &,
                  U &) const {}

} _unify_cospan_with_type;

void unify_cospan_with_type(CospanUnifiers &unified,
                            std::vector<std::size_t> &count,
                            TypeConstructor::Type const &type,
                            CospanMorphism::Type &cospan_type) {
  return std::visit(std::bind(_unify_cospan_with_type, std::ref(unified),
                              std::ref(count), std::placeholders::_1,
                              std::placeholders::_2),
                    type, cospan_type);
}

template <typename T>
void unify_cospan_with_type_with(CospanUnifiers &unified,
                                 std::vector<std::size_t> &count, T const &type,
                                 CospanMorphism::Type &cospan_type) {
  return std::visit(std::bind(_unify_cospan_with_type, std::ref(unified),
                              std::ref(count), std::cref(type),
                              std::placeholders::_1),
                    cospan_type);
}

template <typename T>
void unify_cospan_with_type_with(CospanUnifiers &unified,
                                 std::vector<std::size_t> &count,
                                 TypeConstructor::Type const &type,
                                 T &cospan_type) {
  return std::visit(std::bind(_unify_cospan_with_type, std::ref(unified),
                              std::ref(count), std::placeholders::_1,
                              std::ref(cospan_type)),
                    type);
}

template <typename T>
std::vector<std::vector<T>> create_vector(std::size_t outer_size,
                                          std::size_t inner_size,
                                          T const &default_value) {
  std::vector<std::vector<T>> outer;
  outer.reserve(outer_size);
  for (auto i = 0u; i < outer_size; ++i)
    outer.emplace_back(std::vector<T>(inner_size, default_value));
  return std::move(outer);
}

} // namespace

namespace Project {
namespace Naturality {

std::vector<std::size_t>
unify_cospan_with_type(NaturalTransformation const &transformation,
                       CospanStructure &cospan) {
  CospanUnifiers unified = create_vector(
      transformation.symbols.size(), cospan.total_number_of_identifiers,
      std::optional<std::size_t>(std::nullopt));
  std::vector<std::size_t> count(transformation.symbols.size(), 0);

  for (auto i = 0u; i < transformation.domains.size(); ++i)
    unify_constructor_with_morphism(unified, count, transformation.domains[i],
                                    cospan.domains[i]);
  return std::move(count);
}

} // namespace Naturality
} // namespace Project

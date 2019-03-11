#include "naturality/cospan_substitution.hpp"
#include "naturality/cospan_equality.hpp"
#include "naturality/cospan_zip.hpp"
#include "polymorphic_types/type_equality.hpp"

namespace {

using namespace Project::Naturality;
using namespace Project::Types;

template <typename T>
CospanMorphism::Type
apply_unification(VariableSubstitution &,
                  std::vector<std::optional<TypeConstructor::Type>> const &,
                  T const &, TypeConstructor::Type const &);

CospanMorphism::Type apply_unification_to_type(
    VariableSubstitution &,
    std::vector<std::optional<TypeConstructor::Type>> const &,
    CospanMorphism::Type const &, TypeConstructor::Type const &);

CospanMorphism::Type add_cospan_substitution(
    VariableSubstitution &substitutions,
    std::vector<std::optional<TypeConstructor::Type>> const &unification,
    std::size_t cospan_value, std::size_t identifier) {
  auto &cospan_substitution = substitutions[identifier];

  if (auto substitution = cospan_substitution.values[cospan_value])
    return std::move(*substitution);
  else if (auto type_substitution = unification[identifier])
    return *(cospan_substitution.values[cospan_value] = apply_unification(
                 substitutions, unification, cospan_value, *type_substitution));

  return *(cospan_substitution.values[cospan_value] =
               cospan_substitution.maximum++);
}

CospanMorphism add_cospan_substitution(
    VariableSubstitution &substitutions,
    std::vector<std::optional<TypeConstructor::Type>> const &unification,
    std::vector<CospanMorphism::MappedType> const &morphism,
    TypeConstructor::ConstructorType const &constructor) {
  CospanMorphism substituted;
  substituted.map.reserve(morphism.size());
  for (auto i = 0u; i < morphism.size(); ++i) {
    auto const &cospan_type = morphism[i];
    auto const &substituted_type = apply_unification_to_type(
        substitutions, unification, cospan_type.type, constructor[i].type);
    substituted.map.emplace_back(
        CospanMorphism::MappedType{substituted_type, cospan_type.variance});
  }
  return std::move(substituted);
}

CospanMorphism add_cospan_substitution(
    VariableSubstitution &substitutions,
    std::vector<std::optional<TypeConstructor::Type>> const &unification,
    CospanMorphism const &morphism, TypeConstructor const &constructor) {
  auto const &nested_morphism = get_nested(morphism);
  auto const &nested_constructor = get_nested(constructor);
  auto const expected_size = nested_morphism.map.size();

  if (nested_constructor.type.size() != expected_size)
    throw std::runtime_error("cospan and type constructor do not match");
  return add_cospan_substitution(substitutions, unification, morphism.map,
                                 constructor.type);
}

CospanMorphism::Type add_cospan_substitution(
    VariableSubstitution &substitutions,
    std::vector<std::optional<TypeConstructor::Type>> const &unification,
    CospanMorphism const &morphism, FunctorTypeConstructor const &functor) {
  auto const &nested_morphism = get_nested(morphism);
  auto const expected_size = nested_morphism.map.size();

  if (functor.type.size() != expected_size)
    throw std::runtime_error("cospan and type constructor do not match");
  return add_cospan_substitution(substitutions, unification, morphism.map,
                                 functor.type);
}

struct ApplyUnificationToType {

  CospanMorphism::Type operator()(
      VariableSubstitution &substitutions,
      std::vector<std::optional<TypeConstructor::Type>> const &unification,
      std::size_t cospan_value, std::size_t identifier) const {
    return add_cospan_substitution(substitutions, unification, cospan_value,
                                   identifier);
  }

  CospanMorphism::Type operator()(
      VariableSubstitution &substitutions,
      std::vector<std::optional<TypeConstructor::Type>> const &unification,
      CospanMorphism::PairType const &cospan_value,
      std::size_t identifier) const {
    return zip_cospan_types(
        add_cospan_substitution(substitutions, unification, cospan_value.first,
                                identifier),
        add_cospan_substitution(substitutions, unification, cospan_value.second,
                                identifier));
  }

  CospanMorphism::Type operator()(
      VariableSubstitution &substitutions,
      std::vector<std::optional<TypeConstructor::Type>> const &unification,
      CospanMorphism const &morphism,
      TypeConstructor const &constructor) const {
    return add_cospan_substitution(substitutions, unification, morphism,
                                   constructor);
  }

  CospanMorphism::Type operator()(
      VariableSubstitution &substitutions,
      std::vector<std::optional<TypeConstructor::Type>> const &unification,
      CospanMorphism const &morphism,
      FunctorTypeConstructor const &functor) const {
    return add_cospan_substitution(substitutions, unification, morphism,
                                   functor);
  }

  CospanMorphism::Type operator()(
      VariableSubstitution &substitutions,
      std::vector<std::optional<TypeConstructor::Type>> const &unification,
      std::size_t cospan_value, TypeConstructor const &constructor) const {
    if (auto const identifier = get_identifier(constructor))
      return add_cospan_substitution(substitutions, unification, cospan_value,
                                     *identifier);
    throw std::runtime_error("cospan and type constructor do not match");
  }

  CospanMorphism::Type operator()(
      VariableSubstitution &substitutions,
      std::vector<std::optional<TypeConstructor::Type>> const &unification,
      CospanMorphism const &morphism, std::size_t identifier) const {
    if (auto const cospan_value = get_identifier(morphism))
      return add_cospan_substitution(substitutions, unification, *cospan_value,
                                     identifier);
    throw std::runtime_error("cospan and type constructor do not match");
  }

  template <typename T>
  CospanMorphism::Type operator()(
      VariableSubstitution &substitutions,
      std::vector<std::optional<TypeConstructor::Type>> const &unification,
      T const &, FunctorTypeConstructor const &) const {
    throw std::runtime_error("cospan and type constructor do not match");
  }

  template <typename T>
  CospanMorphism::Type operator()(
      VariableSubstitution &substitutions,
      std::vector<std::optional<TypeConstructor::Type>> const &unification,
      T const &, TypeConstructor const &) const {
    throw std::runtime_error("cospan and type constructor do not match");
  }

  template <typename T>
  CospanMorphism::Type operator()(
      VariableSubstitution &substitutions,
      std::vector<std::optional<TypeConstructor::Type>> const &unification,
      CospanMorphism const &, T const &) const {
    throw std::runtime_error("cospan and type constructor do not match");
  }

  template <typename T, typename U>
  CospanMorphism::Type
  operator()(VariableSubstitution &,
             std::vector<std::optional<TypeConstructor::Type>> const &,
             T const &, U const &) const {
    return 0;
  }

} _apply_unification_to_type;

template <typename T>
CospanMorphism::Type apply_unification(
    VariableSubstitution &substitution,
    std::vector<std::optional<TypeConstructor::Type>> const &unification,
    T const &cospan_type, TypeConstructor::Type const &type) {
  return std::visit(std::bind(_apply_unification_to_type,
                              std::ref(substitution), std::cref(unification),
                              std::cref(cospan_type), std::placeholders::_1),
                    type);
}

CospanMorphism::Type apply_unification_to_type(
    VariableSubstitution &substitution,
    std::vector<std::optional<TypeConstructor::Type>> const &unification,
    CospanMorphism::Type const &cospan_type,
    TypeConstructor::Type const &type) {
  return std::visit(std::bind(_apply_unification_to_type,
                              std::ref(substitution), std::cref(unification),
                              std::placeholders::_1, std::placeholders::_2),
                    cospan_type, type);
}

} // namespace

namespace Project {
namespace Naturality {

CospanMorphism cospan_substitution(
    VariableSubstitution &substitutions,
    std::vector<std::optional<Types::TypeConstructor::Type>> const &unification,
    CospanMorphism const &morphism, Types::TypeConstructor const &constructor) {
  return add_cospan_substitution(substitutions, unification, morphism,
                                 constructor);
}

VariableSubstitution
create_empty_substitution(CospanStructure const &cospan,
                          NaturalTransformation const &transformation) {
  auto const identifiers = transformation.symbols.size();
  auto const cospan_values = cospan.number_of_identifiers;

  VariableSubstitution substitution;
  substitution.reserve(identifiers);
  for (auto i = 0u; i < identifiers; ++i)
    substitution.emplace_back(
        CospanSubstitutions{std::vector<std::optional<CospanMorphism::Type>>(
                                cospan_values, std::nullopt),
                            0});
  return std::move(substitution);
}

} // namespace Naturality
} // namespace Project
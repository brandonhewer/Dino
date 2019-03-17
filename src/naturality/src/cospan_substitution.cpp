#include "naturality/cospan_substitution.hpp"
#include "naturality/cospan_equality.hpp"
#include "naturality/cospan_zip.hpp"
#include "polymorphic_types/type_equality.hpp"

#include <iostream>

namespace {

using namespace Project::Naturality;
using namespace Project::Types;

CospanMorphism::Type create_new_cospan_type(std::vector<std::size_t> &,
                                            TypeConstructor::Type const &);

std::vector<CospanMorphism::MappedType>
create_new_cospan_type(std::vector<std::size_t> &index,
                       TypeConstructor::ConstructorType const &constructor) {
  std::vector<CospanMorphism::MappedType> morphism;
  morphism.reserve(constructor.size());
  for (auto &&type : constructor)
    morphism.emplace_back(CospanMorphism::MappedType{
        create_new_cospan_type(index, type.type), type.variance});
  return std::move(morphism);
}

CospanMorphism create_new_cospan_type(std::vector<std::size_t> &index,
                                      FunctorTypeConstructor const &functor) {
  return {create_new_cospan_type(index, functor.type)};
}

CospanMorphism create_new_cospan_type(std::vector<std::size_t> &index,
                                      TypeConstructor const &constructor) {
  return {create_new_cospan_type(index, constructor.type)};
}

struct CreateNewCospanType {
  CospanMorphism::Type operator()(std::vector<std::size_t> &index,
                                  std::size_t identifier) const {
    return index[identifier]++;
  }

  CospanMorphism::Type operator()(std::vector<std::size_t> &, FreeType) const {
    return 0;
  }

  CospanMorphism::Type operator()(std::vector<std::size_t> &, MonoType) const {
    return 0;
  }

  CospanMorphism::Type operator()(std::vector<std::size_t> &index,
                                  FunctorTypeConstructor const &functor) const {
    return create_new_cospan_type(index, functor);
  }

  CospanMorphism::Type operator()(std::vector<std::size_t> &index,
                                  TypeConstructor const &constructor) const {
    return create_new_cospan_type(index, constructor);
  }

} _create_new_cospan_type;

CospanMorphism::Type create_new_cospan_type(std::vector<std::size_t> &index,
                                            TypeConstructor::Type const &type) {
  return std::visit(std::bind(_create_new_cospan_type, std::ref(index),
                              std::placeholders::_1),
                    type);
}

CospanMorphism::Type apply_unification_to_type(
    VariableSubstitution &,
    std::vector<std::optional<TypeConstructor::Type>> const &,
    CospanMorphism::Type const &, TypeConstructor::Type const &);

template <typename T>
CospanMorphism::Type apply_unification_to_type_with(
    VariableSubstitution &,
    std::vector<std::optional<TypeConstructor::Type>> const &, T const &,
    TypeConstructor::Type const &);

template <typename T>
CospanMorphism::Type apply_unification_to_type_with(
    VariableSubstitution &,
    std::vector<std::optional<TypeConstructor::Type>> const &,
    CospanMorphism::Type const &, T const &);

CospanMorphism::Type add_cospan_substitution(
    VariableSubstitution &substitutions,
    std::vector<std::optional<TypeConstructor::Type>> const &unification,
    std::size_t cospan_value, std::size_t identifier) {
  auto &cospan_substitution = substitutions[identifier];

  if (auto substitution = cospan_substitution.values[cospan_value])
    return std::move(*substitution);
  else if (auto type_substitution = unification[identifier])
    return *(cospan_substitution.values[cospan_value] = create_new_cospan_type(
                 cospan_substitution.maximum, *type_substitution));

  return *(cospan_substitution.values[cospan_value] =
               cospan_substitution.maximum[identifier]++);
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

CospanMorphism::Type add_cospan_substitution(
    VariableSubstitution &substitutions,
    std::vector<std::optional<TypeConstructor::Type>> const &unification,
    CospanMorphism const &morphism, TypeConstructor const &constructor) {
  auto const &nested_morphism = get_nested(morphism);
  auto const &nested_constructor = get_nested(constructor);
  auto const constructor_size = nested_constructor.type.size();
  auto const morphism_size = nested_morphism.map.size();

  if (constructor_size == morphism_size)
    return add_cospan_substitution(substitutions, unification,
                                   nested_morphism.map,
                                   nested_constructor.type);
  else if (morphism_size == 1)
    return apply_unification_to_type_with(
        substitutions, unification, nested_morphism.map[0], nested_constructor);
  else if (constructor_size == 1)
    return apply_unification_to_type_with(substitutions, unification,
                                          nested_morphism,
                                          nested_constructor.type[0].type);
  throw std::runtime_error(
      "cospan and type constructor do not match (constructor size)");
}

CospanMorphism::Type add_cospan_substitution(
    VariableSubstitution &substitutions,
    std::vector<std::optional<TypeConstructor::Type>> const &unification,
    CospanMorphism const &morphism, FunctorTypeConstructor const &functor) {
  auto const &nested_morphism = get_nested(morphism);
  auto const functor_size = functor.type.size();
  auto const morphism_size = nested_morphism.map.size();

  if (functor_size == morphism_size)
    return add_cospan_substitution(substitutions, unification,
                                   nested_morphism.map, functor.type);
  else if (morphism_size == 1)
    return apply_unification_to_type_with(substitutions, unification,
                                          nested_morphism.map[0], functor);
  else if (functor_size == 1)
    return apply_unification_to_type_with(
        substitutions, unification, nested_morphism, functor.type[0].type);
  throw std::runtime_error(
      "cospan and type constructor do not match (functor size)");
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

  template <typename T>
  CospanMorphism::Type operator()(
      VariableSubstitution &substitutions,
      std::vector<std::optional<TypeConstructor::Type>> const &unification,
      T const &type, FunctorTypeConstructor const &functor) const {
    if (functor.type.size() == 1)
      return apply_unification_to_type_with(substitutions, unification, type,
                                            functor.type[0].type);
    throw std::runtime_error(
        "cospan and type constructor do not match (functor)");
  }

  template <typename T>
  CospanMorphism::Type operator()(
      VariableSubstitution &substitutions,
      std::vector<std::optional<TypeConstructor::Type>> const &unification,
      T const &type, TypeConstructor const &constructor) const {
    if (constructor.type.size() == 1)
      return apply_unification_to_type_with(substitutions, unification, type,
                                            constructor.type[0].type);
    throw std::runtime_error(
        "cospan and type consstructor do not match (constructor)");
  }

  template <typename T>
  CospanMorphism::Type operator()(
      VariableSubstitution &substitutions,
      std::vector<std::optional<TypeConstructor::Type>> const &unification,
      CospanMorphism const &morphism, T const &type) const {
    if (morphism.map.size() == 1)
      return apply_unification_to_type_with(substitutions, unification,
                                            morphism.map[0], type);
    throw std::runtime_error(
        "cospan and type constructor do not match (morphism)");
  }

  template <typename T, typename U>
  CospanMorphism::Type
  operator()(VariableSubstitution &,
             std::vector<std::optional<TypeConstructor::Type>> const &,
             T const &, U const &) const {
    return 0;
  }

} _apply_unification_to_type;

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

template <typename T>
CospanMorphism::Type apply_unification_to_type_with(
    VariableSubstitution &substitution,
    std::vector<std::optional<TypeConstructor::Type>> const &unification,
    T const &cospan_type, TypeConstructor::Type const &type) {
  return std::visit(std::bind(_apply_unification_to_type,
                              std::ref(substitution), std::cref(unification),
                              std::cref(cospan_type), std::placeholders::_1),
                    type);
}

template <typename T>
CospanMorphism::Type apply_unification_to_type_with(
    VariableSubstitution &substitution,
    std::vector<std::optional<TypeConstructor::Type>> const &unification,
    CospanMorphism::Type const &cospan_type, T const &type) {
  return std::visit(std::bind(_apply_unification_to_type,
                              std::ref(substitution), std::cref(unification),
                              std::placeholders::_1, std::cref(type)),
                    cospan_type);
}

struct MoveToCospanMorphism {

  CospanMorphism operator()(CospanMorphism &&morphism) const {
    return std::move(morphism);
  }

  template <typename T> CospanMorphism operator()(T &&type) const {
    return {std::vector<CospanMorphism::MappedType>{
        {std::move(type), Variance::COVARIANCE}}};
  }

} _move_to_cospan_morphism;

CospanMorphism move_to_cospan_morphism(CospanMorphism::Type &&type) {
  return std::visit(_move_to_cospan_morphism, std::move(type));
}

} // namespace

namespace Project {
namespace Naturality {

CospanMorphism cospan_substitution(
    VariableSubstitution &substitutions,
    std::vector<std::optional<Types::TypeConstructor::Type>> const &unification,
    CospanMorphism const &morphism, Types::TypeConstructor const &constructor) {
  return move_to_cospan_morphism(add_cospan_substitution(
      substitutions, unification, morphism, constructor));
}

VariableSubstitution
create_empty_substitution(CospanStructure const &cospan,
                          NaturalTransformation const &transformation,
                          std::size_t number_of_identifiers) {
  return create_empty_substitution(
      cospan, transformation,
      std::vector<std::size_t>(number_of_identifiers, 0));
}

VariableSubstitution
create_empty_substitution(CospanStructure const &cospan,
                          NaturalTransformation const &transformation,
                          std::vector<std::size_t> const &identifier_counts) {
  auto const identifiers = transformation.symbols.size();
  auto const cospan_values = cospan.total_number_of_identifiers;

  VariableSubstitution substitution;
  substitution.reserve(identifiers);
  for (auto i = 0u; i < identifiers; ++i)
    substitution.emplace_back(
        CospanSubstitutions{std::vector<std::optional<CospanMorphism::Type>>(
                                cospan_values, std::nullopt),
                            identifier_counts});
  return std::move(substitution);
}

} // namespace Naturality
} // namespace Project
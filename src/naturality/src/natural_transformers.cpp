#include "naturality/natural_transformers.hpp"

namespace {

using namespace Project::Naturality;
using namespace Project::Types;

bool is_integer_compatible(TypeConstructor const &);

struct IsTypeIntegerCompatible {

  bool operator()(std::size_t) const { return true; }

  bool operator()(TypeConstructor const &constructor) const {
    return is_integer_compatible(constructor);
  }

  template <typename T> bool operator()(T const &) const { return false; }
} _is_type_integer_compatible;

bool is_integer_compatible(TypeConstructor const &constructor) {
  return constructor.type.size() == 1 &&
         std::visit(_is_type_integer_compatible, constructor.type[0].type);
}

bool is_integer_compatible(CospanMorphism const &);

struct IsCospanIntegerCompatible {

  bool operator()(std::size_t) const { return true; }

  bool operator()(CospanMorphism const &morphism) const {
    return is_integer_compatible(morphism);
  }

  template <typename T> bool operator()(T const &) const { return false; }

} _is_cospan_integer_compatible;

bool is_integer_compatible(CospanMorphism const &morphism) {
  return morphism.map.size() == 1 &&
         std::visit(_is_cospan_integer_compatible, morphism.map[0].type);
}

std::optional<CompatibilityError>
is_cospan_incompatible_with(std::size_t identifier,
                            CospanMorphism const &morphism) {
  if (!is_integer_compatible(morphism))
    return StructureError{identifier, morphism};
  return std::nullopt;
}

std::optional<CompatibilityError>
is_type_incompatible_with(TypeConstructor const &constructor,
                          std::size_t cospan_value) {
  if (!is_integer_compatible(constructor))
    return StructureError{constructor, cospan_value};
  return std::nullopt;
}

std::optional<CompatibilityError>
is_type_incompatible(TypeConstructor::ConstructorType const &,
                     std::vector<CospanMorphism::MappedType> const &);

struct IsIncompatible {

  std::optional<CompatibilityError> operator()(std::size_t, std::size_t) const {
    return std::nullopt;
  }

  std::optional<CompatibilityError>
  operator()(std::size_t, CospanMorphism::PairType const &) const {
    return std::nullopt;
  }

  std::optional<CompatibilityError>
  operator()(TypeConstructor const &type,
             CospanMorphism const &morphism) const {
    return is_type_incompatible(type.type, morphism.map);
  }

  std::optional<CompatibilityError>
  operator()(FunctorTypeConstructor const &functor,
             CospanMorphism const &morphism) const {
    return is_type_incompatible(functor.type, morphism.map);
  }

  std::optional<CompatibilityError>
  operator()(std::size_t identifier, CospanMorphism const &morphism) const {
    return is_cospan_incompatible_with(identifier, morphism);
  }

  std::optional<CompatibilityError> operator()(TypeConstructor const &type,
                                               std::size_t cospan_value) const {
    return is_type_incompatible_with(type, cospan_value);
  }

  template <typename T>
  std::optional<CompatibilityError> operator()(std::size_t identifier,
                                               T const &cospan) const {
    return StructureError{identifier, cospan};
  }

  template <typename T>
  std::optional<CompatibilityError> operator()(TypeConstructor const &type,
                                               T const &cospan) const {
    return StructureError{type, cospan};
  }

  template <typename T>
  std::optional<CompatibilityError>
  operator()(FunctorTypeConstructor const &functor, T const &cospan) const {
    return StructureError{functor, cospan};
  }

  template <typename T>
  std::optional<CompatibilityError>
  operator()(T const &type, CospanMorphism const &morphism) const {
    return StructureError{type, morphism};
  }

  template <typename T, typename U>
  std::optional<CompatibilityError> operator()(T const &, U const &) {
    return std::nullopt;
  }

} _is_incompatible;

std::optional<CompatibilityError>
is_type_incompatible(TypeConstructor::Type const &type,
                     CospanMorphism::Type const &cospan_type) {
  return std::visit(_is_incompatible, type, cospan_type);
}

std::optional<CompatibilityError>
is_type_incompatible(TypeConstructor::AtomicType const &type,
                     CospanMorphism::MappedType const &cospan_type) {
  if (type.variance != cospan_type.variance)
    return VarianceError{type, cospan_type};
  return is_type_incompatible(type.type, cospan_type.type);
}

std::optional<CompatibilityError> is_type_incompatible(
    TypeConstructor::ConstructorType const &type,
    std::vector<CospanMorphism::MappedType> const &cospan_type) {
  if (type.size() != cospan_type.size())
    return StructureError{TypeConstructor{type}, CospanMorphism{cospan_type}};

  for (auto i = 0u; i < type.size(); ++i) {
    if (auto error = is_type_incompatible(type[i], cospan_type[i]))
      return std::move(error);
  }
  return std::nullopt;
}

} // namespace

namespace Project {
namespace Naturality {

std::optional<CompatibilityError>
is_incompatible(Types::TypeConstructor const &type,
                CospanMorphism const &morphism) {
  return is_type_incompatible(type.type, morphism.map);
}

std::optional<CompatibilityError>
is_incompatible(NaturalTransformation const &transformation,
                CospanStructure const &cospan) {
  for (auto i = 0u; i < transformation.domains.size(); ++i) {
    if (auto error =
            is_incompatible(transformation.domains[i], cospan.domains[i]))
      return std::move(error);
  }
  return std::nullopt;
}

} // namespace Naturality
} // namespace Project

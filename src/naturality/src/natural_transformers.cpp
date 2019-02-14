#include "naturality/natural_transformers.hpp"

namespace {

using namespace Project::Naturality;
using namespace Project::Types;

std::optional<CompatibilityError>
is_type_incompatible(TypeConstructor::ConstructorType const &,
                     std::vector<CospanMorphism::MappedType> const &);

struct IsCompatible {

  std::optional<CompatibilityError> operator()(std::size_t, std::size_t) const {
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

} _is_compatible;

std::optional<CompatibilityError>
is_type_incompatible(TypeConstructor::Type const &type,
                     CospanMorphism::Type const &cospan_type) {
  return std::visit(_is_compatible, type, cospan_type);
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
  if (auto error = is_incompatible(transformation.domain, cospan.domain))
    return std::move(error);
  else if (auto error =
               is_incompatible(transformation.codomain, cospan.codomain))
    return std::move(error);
  return std::nullopt;
}

} // namespace Naturality
} // namespace Project

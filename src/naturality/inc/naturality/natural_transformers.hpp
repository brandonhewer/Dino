#ifndef __NATURAL_TRANSFORMERS_HPP_
#define __NATURAL_TRANSFORMERS_HPP_

#include "naturality/cospan.hpp"
#include "naturality/natural_transformation.hpp"

#include <optional>
#include <variant>

namespace Project {
namespace Naturality {

struct StructureError {
  Types::TypeConstructor::Type type;
  CospanMorphism::Type cospan_type;
};

struct VarianceError {
  Types::TypeConstructor::AtomicType type;
  CospanMorphism::MappedType cospan_type;
};

using CompatibilityError = std::variant<StructureError, VarianceError>;

std::optional<CompatibilityError>
is_incompatible(Types::TypeConstructor const &type,
                CospanMorphism const &morphism);

std::optional<CompatibilityError>
is_incompatible(NaturalTransformation const &transformation,
                CospanStructure const &cospan);

} // namespace Naturality
} // namespace Project

#endif

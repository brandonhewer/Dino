#include "polymorphic_types/type_transformers.hpp"
#include "polymorphic_types/type_errors.hpp"

#include <node_api.h>

namespace {

using namespace Project::Types;

TypeConstructor::ConstructorType
compose_constructors(TypeConstructor::ConstructorType const &lhs,
                     TypeConstructor::ConstructorType const &rhs) {
  if (lhs.empty())
    return rhs;
  else if (rhs.empty())
    return lhs;

  auto const &lhs_variance = lhs.back().variance;
  auto const &rhs_variance = rhs.front().variance;
  if (lhs_variance == rhs_variance)
    throw CompositionError(lhs_variance, rhs_variance);

  TypeConstructor::ConstructorType composed;
  composed.insert(composed.end(), lhs.begin(), lhs.end() - 1);
  composed.insert(composed.end(), rhs.begin() + 1, rhs.end());
  return std::move(composed);
}

} // namespace

namespace Project {
namespace Types {

TypeConstructor compose_type_constructors(TypeConstructor const &lhs,
                                          TypeConstructor const &rhs) {
  return TypeConstructor{compose_constructors(lhs.type, rhs.type)};
}

} // namespace Types
} // namespace Project

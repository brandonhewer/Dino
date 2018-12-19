#include "polymorphic_types/type_errors.hpp"
#include "polymorphic_types/types.hpp"

#include <string>

namespace {
using namespace Project::Types;

std::string variance_to_string(Variance variance) {
  switch (variance) {
  case Variance::CONTRAVARIANCE:
    return "contravariant";
  case Variance::COVARIANCE:
    return "covariant";
  }
}

std::string variance_error_message(Variance lhs, Variance rhs) {
  return variance_to_string(lhs) + " type variable is not composable with a " +
         variance_to_string(rhs) + " type variable";
}

} // namespace

namespace Project {
namespace Types {

CompositionError::CompositionError(Variance lhs, Variance rhs)
    : std::runtime_error(variance_error_message(lhs, rhs)) {}

EmptyTypeConstructorError::EmptyTypeConstructorError()
    : std::runtime_error("empty type con") {}

} // namespace Types
} // namespace Project

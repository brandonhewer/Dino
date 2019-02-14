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
  case Variance::INVARIANCE:
    return "invariant";
  case Variance::BIVARIANCE:
    return "bivariant";
  }
}

std::string variance_error_message(Variance lhs, Variance rhs) {
  return variance_to_string(lhs) + " type variable is not composable with a " +
         variance_to_string(rhs) + " type variable";
}

std::string identifier_error_message(std::size_t lhs, std::size_t rhs) {
  return "type parameter {" + std::to_string(lhs) + "}";
}

} // namespace

namespace Project {
namespace Types {

CompositionError::CompositionError(Variance lhs, Variance rhs)
    : std::runtime_error(variance_error_message(lhs, rhs)) {}

TypeMismatchError::TypeMismatchError(TypeConstructor::Type const &lhs,
                                     TypeConstructor::Type const &rhs)
    : std::exception(), m_lhs(lhs), m_rhs(rhs) {}

TypeConstructor::Type const &TypeMismatchError::left() const { return m_lhs; }

TypeConstructor::Type const &TypeMismatchError::right() const { return m_rhs; }

InvalidTypeAccessError::InvalidTypeAccessError()
    : std::runtime_error("attempted to access type at an invalid position") {}

EmptyTypeConstructorError::EmptyTypeConstructorError()
    : std::runtime_error("empty type constructor") {}

} // namespace Types
} // namespace Project

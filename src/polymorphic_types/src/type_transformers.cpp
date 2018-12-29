#include "polymorphic_types/type_transformers.hpp"

namespace {

using namespace Project::Types;

struct ThrowIfInvalidComposition {

  template <typename T> void operator()(T const &lhs, T const &rhs) const {
    if (lhs != rhs)
      throw TypeMismatchError(lhs, rhs);
  }

  template <typename T, typename U>
  void operator()(T const &lhs, U const &rhs) const {
    throw TypeMismatchError(lhs, rhs);
  }

} _throw_if_invalid_composition;

void throw_if_invalid_composition(TypeConstructor::AtomicType const &lhs,
                                  TypeConstructor::AtomicType const &rhs) {
  if (lhs.variance == rhs.variance)
    throw CompositionError(lhs.variance, rhs.variance);
  std::visit(_throw_if_invalid_composition, lhs.type, rhs.type);
}

TypeConstructor::ConstructorType
compose_constructors(TypeConstructor::ConstructorType const &lhs,
                     TypeConstructor::ConstructorType const &rhs) {
  if (lhs.empty())
    return rhs;
  else if (rhs.empty())
    return lhs;

  throw_if_invalid_composition(lhs.back(), rhs.front());

  TypeConstructor::ConstructorType composed;
  composed.insert(composed.end(), lhs.begin(), lhs.end() - 1);
  composed.insert(composed.end(), rhs.begin() + 1, rhs.end());
  return std::move(composed);
}

struct ExtractConstructorType {

  TypeConstructor::ConstructorType &
  operator()(TypeConstructor &constructor) const {
    return constructor.type;
  }

  template <typename T>
  TypeConstructor::ConstructorType &operator()(T &) const {
    throw InvalidTypeAccessError();
  }
} _extract_constructor_type;

} // namespace

namespace Project {
namespace Types {

TypeConstructor::ConstructorType &
extract_constructor_type(TypeConstructor::AtomicType const &type) {
  return std::visit(_extract_constructor_type, type.type);
}

bool is_composable(TypeConstructor const &lhs, TypeConstructor const &rhs) {
  if (lhs.type.empty() || rhs.type.empty())
    return true;

  try {
    throw_if_invalid_composition(lhs.type.back(), rhs.type.front());
    return true;
  } catch (std::exception &) {
    return false;
  }
}

TypeConstructor compose_type_constructors(TypeConstructor const &lhs,
                                          TypeConstructor const &rhs) {
  return TypeConstructor{compose_constructors(lhs.type, rhs.type)};
}

} // namespace Types
} // namespace Project

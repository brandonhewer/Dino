#include "polymorphic_types/type_transformers.hpp"

namespace {

using namespace Project::Types;

bool is_valid_composition(TypeConstructor const &, TypeConstructor const &);

struct IsValidComposition {

  bool operator()(FreeType const &, FreeType const &) const { return true; }

  bool operator()(MonoType const &lhs, MonoType const &rhs) const {
    return lhs == rhs;
  }

  bool operator()(std::size_t, std::size_t) const { return true; }

  bool operator()(TypeConstructor const &lhs,
                  TypeConstructor const &rhs) const {
    return is_valid_composition(lhs, rhs);
  }

  bool operator()(TypeConstructor const &, std::size_t) const { return true; }

  template <typename T, typename U>
  bool operator()(T const &lhs, U const &rhs) const {
    return false;
  }

} _is_valid_composition;

bool is_valid_composition(TypeConstructor const &lhs,
                          TypeConstructor const &rhs) {
  for (auto i = 0u; i < lhs.type.size(); ++i) {
    auto const &lhs_type = lhs.type[i];
    auto const &rhs_type = rhs.type[i];
    if (lhs_type.variance != rhs_type.variance ||
        !std::visit(_is_valid_composition, lhs_type.type, rhs_type.type))
      return false;
  }
  return true;
}

void throw_if_invalid_composition(TypeConstructor::AtomicType const &lhs,
                                  TypeConstructor::AtomicType const &rhs) {
  if (lhs.variance == rhs.variance)
    throw CompositionError(lhs.variance, rhs.variance);
  if (!std::visit(_is_valid_composition, lhs.type, rhs.type))
    throw TypeMismatchError(lhs.type, rhs.type);
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

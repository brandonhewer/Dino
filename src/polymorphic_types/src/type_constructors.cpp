#include "polymorphic_types/type_constructors.hpp"

namespace {

using namespace Project::Types;

template <typename... Ts>
TypeConstructor::ConstructorType constructor_type(Ts &&... types) noexcept {
  return TypeConstructor::ConstructorType{std::forward<Ts>(types)...};
}

template <typename T>
TypeConstructor::ConstructorType constructor_type(std::size_t degree,
                                                  T &&type) noexcept {
  return TypeConstructor::ConstructorType(degree, std::forward<T>(type));
}

template <typename... Ts> TypeConstructor create_type(Ts &&... types) noexcept {
  return TypeConstructor{constructor_type(std::forward<Ts>(types)...)};
}

template <typename T>
TypeConstructor create_type(std::size_t degree, T &&type) noexcept {
  return {constructor_type(degree, std::forward<T>(type))};
}

} // namespace

namespace Project {
namespace Types {

TypeConstructor::AtomicType create_covariant_type_parameter() noexcept {
  return TypeConstructor::AtomicType{FreeType{}, Variance::COVARIANCE};
}

TypeConstructor::AtomicType create_contravariant_type_parameter() noexcept {
  return TypeConstructor::AtomicType{FreeType{}, Variance::CONTRAVARIANCE};
}

TypeConstructor create_function_type_constructor() noexcept {
  return create_type(create_contravariant_type_parameter(),
                     create_covariant_type_parameter());
}

TypeConstructor create_product_type_constructor(std::size_t degree) noexcept {
  return create_type(degree, create_covariant_type_parameter());
}

} // namespace Types
} // namespace Project
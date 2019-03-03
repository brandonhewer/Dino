#include "polymorphic_types/type_constructor.hpp"

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

TypeConstructor::ConstructorType
extract_tail(TypeConstructor::ConstructorType const &constructor,
             std::size_t from) {
  TypeConstructor::ConstructorType tail;
  tail.reserve(constructor.size() - from);
  std::copy(constructor.begin() + from, constructor.end(),
            std::back_inserter(tail));
  return std::move(tail);
}

} // namespace

namespace Project {
namespace Types {

TypeConstructor tail_constructor(TypeConstructor const &constructor,
                                 std::size_t from) {
  return {extract_tail(constructor.type, from)};
}

TypeConstructor::AtomicType create_covariant_type_parameter() noexcept {
  return TypeConstructor::AtomicType{FreeType{}, Variance::COVARIANCE};
}

TypeConstructor::AtomicType create_contravariant_type_parameter() noexcept {
  return TypeConstructor::AtomicType{FreeType{}, Variance::CONTRAVARIANCE};
}

TypeConstructor::AtomicType create_covariant_functor_constructor(
    std::size_t identifier, TypeConstructor::ConstructorType &&types) noexcept {
  return TypeConstructor::AtomicType{
      FunctorTypeConstructor{std::move(types), identifier},
      Variance::COVARIANCE};
}

TypeConstructor::AtomicType create_contravariant_functor_constructor(
    std::size_t identifier, TypeConstructor::ConstructorType &&types) noexcept {
  return TypeConstructor::AtomicType{
      FunctorTypeConstructor{std::move(types), identifier},
      Variance::CONTRAVARIANCE};
}

TypeConstructor create_covariant_type_constructor() noexcept {
  return {TypeConstructor::ConstructorType{create_covariant_type_parameter()}};
}

TypeConstructor create_contravariant_type_constructor() noexcept {
  return {
      TypeConstructor::ConstructorType{create_contravariant_type_parameter()}};
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
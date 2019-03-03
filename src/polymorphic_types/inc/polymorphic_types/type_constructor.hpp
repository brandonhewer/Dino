#ifndef __TYPE_CONSTRUCTOR_HPP_
#define __TYPE_CONSTRUCTOR_HPP_

#include <cstddef>
#include <variant>
#include <vector>

#include "polymorphic_types/types.hpp"

namespace Project {
namespace Types {

struct FunctorTypeConstructor;

struct TypeConstructor {
  using Type = std::variant<FreeType, MonoType, std::size_t,
                            FunctorTypeConstructor, TypeConstructor>;
  using AtomicType = TypeWithVariance<Type>;
  using ConstructorType = std::vector<AtomicType>;
  ConstructorType type;
};

struct FunctorTypeConstructor {
  TypeConstructor::ConstructorType type;
  std::size_t identifier;
};

TypeConstructor tail_constructor(TypeConstructor const &, std::size_t);

TypeConstructor::AtomicType create_covariant_type_parameter() noexcept;

TypeConstructor::AtomicType create_contravariant_type_parameter() noexcept;

TypeConstructor::AtomicType create_covariant_functor_constructor(
    std::size_t, TypeConstructor::ConstructorType &&) noexcept;

TypeConstructor::AtomicType create_contravariant_functor_constructor(
    std::size_t, TypeConstructor::ConstructorType &&) noexcept;

TypeConstructor create_covariant_type_constructor() noexcept;

TypeConstructor create_contravariant_type_constructor() noexcept;

TypeConstructor create_function_type_constructor() noexcept;

TypeConstructor create_product_type_constructor(std::size_t) noexcept;

} // namespace Types
} // namespace Project

#endif

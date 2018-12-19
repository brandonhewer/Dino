#ifndef __TYPE_CONSTRUCTOR_HPP_
#define __TYPE_CONSTRUCTOR_HPP_

#include <cstddef>
#include <variant>
#include <vector>

#include "polymorphic_types/types.hpp"

namespace Project {
namespace Types {

struct TypeConstructor {
  using AtomicType = TypeWithVariance<
      std::variant<FreeType, MonoType, std::size_t, TypeConstructor>>;
  using ConstructorType = std::vector<AtomicType>;
  ConstructorType type;
};

TypeConstructor::AtomicType create_covariant_type_parameter() noexcept;
TypeConstructor::AtomicType create_contravariant_type_parameter() noexcept;
TypeConstructor create_covariant_type_constructor() noexcept;
TypeConstructor create_contravariant_type_constructor() noexcept;
TypeConstructor create_function_type_constructor() noexcept;
TypeConstructor create_product_type_constructor(std::size_t) noexcept;

} // namespace Types
} // namespace Project

#endif

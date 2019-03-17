#ifndef __UNIFICATION_HPP_
#define __UNIFICATION_HPP_

#include "polymorphic_types/type_constructor.hpp"

#include <cstddef>
#include <optional>
#include <vector>

namespace Project {
namespace Types {

struct Unification {
  std::vector<std::optional<TypeConstructor::Type>> left;
  std::vector<std::optional<TypeConstructor::Type>> right;
  std::vector<std::optional<std::size_t>> functor_left;
  std::vector<std::optional<std::size_t>> functor_right;
};

std::optional<Unification>
calculate_unification(TypeConstructor const &left, TypeConstructor const &right,
                      std::size_t left_symbols, std::size_t right_symbols,
                      std::size_t left_functor_symbols,
                      std::size_t right_functor_symbols);

} // namespace Types
} // namespace Project

#endif

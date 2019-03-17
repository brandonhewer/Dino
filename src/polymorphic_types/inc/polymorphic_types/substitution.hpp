#ifndef __SUBSTITUTION_HPP_
#define __SUBSTITUTION_HPP_

#include "polymorphic_types/type_constructor.hpp"

#include <cstddef>
#include <optional>
#include <vector>

namespace Project {
namespace Types {

using Substitution = std::vector<std::optional<TypeConstructor::Type>>;
using FunctorSubstitution = std::vector<std::optional<std::size_t>>;

TypeConstructor apply_substitution(TypeConstructor const &,
                                   Substitution const &,
                                   FunctorSubstitution const &);

} // namespace Types
} // namespace Project

#endif

#ifndef __TYPE_REPLACEMENT_HPP_
#define __TYPE_REPLACEMENT_HPP_

#include "polymorphic_types/type_constructor.hpp"

#include <optional>
#include <string>

namespace Project {
namespace Types {

using TypeReplacements = std::vector<std::optional<std::size_t>>;

TypeConstructor::Type &
replace_identifiers(TypeConstructor::Type &type,
                    TypeReplacements const &replacements);

TypeConstructor &replace_identifiers(TypeConstructor &constructor,
                                     TypeReplacements const &replacements);

} // namespace Types
} // namespace Project

#endif
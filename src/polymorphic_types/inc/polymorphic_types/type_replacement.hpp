#ifndef __TYPE_REPLACEMENT_HPP_
#define __TYPE_REPLACEMENT_HPP_

#include "polymorphic_types/type_constructor.hpp"

#include <optional>
#include <string>

namespace Project {
namespace Types {

using Replacements = std::vector<std::optional<std::size_t>>;

TypeConstructor::Type &replace_identifiers(TypeConstructor::Type &type,
                                           Replacements const &replacements);

TypeConstructor &replace_identifiers(TypeConstructor &constructor,
                                     Replacements const &replacements);

} // namespace Types
} // namespace Project

#endif
#ifndef __TYPE_TO_STRING_HPP_
#define __TYPE_TO_STRING_HPP_

#include "polymorphic_types/type_constructor.hpp"
#include "polymorphic_types/type_errors.hpp"

#include <string>

namespace Project {
namespace Types {

std::string to_string(TypeConstructor const &constructor,
                      std::vector<std::string> const &symbols,
                      std::vector<std::string> const &functor_symbols);

std::string to_string(TypeConstructor::Type const &type,
                      std::vector<std::string> const &symbols,
                      std::vector<std::string> const &functor_symbols);

} // namespace Types
} // namespace Project

#endif

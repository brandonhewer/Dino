#ifndef __TYPE_TRANSFORMERS_HPP_
#define __TYPE_TRANSFORMERS_HPP_

#include "polymorphic_types/type_constructor.hpp"
#include "polymorphic_types/type_errors.hpp"

#include <string>

namespace Project {
namespace Types {

TypeConstructor::ConstructorType &
extract_constructor_type(TypeConstructor::AtomicType const &type);

bool is_composable(TypeConstructor const &, TypeConstructor const &);

TypeConstructor compose_type_constructors(TypeConstructor const &,
                                          TypeConstructor const &);

std::string to_string(TypeConstructor const &constructor,
                      std::vector<std::string> const &symbols);

std::string to_string(TypeConstructor::Type const &type,
                      std::vector<std::string> const &symbols);

} // namespace Types
} // namespace Project

#endif

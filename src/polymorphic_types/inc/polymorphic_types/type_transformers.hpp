#ifndef __TYPE_TRANSFORMERS_HPP_
#define __TYPE_TRANSFORMERS_HPP_

#include "polymorphic_types/type_constructor.hpp"
#include "polymorphic_types/type_errors.hpp"

namespace Project {
namespace Types {

TypeConstructor::ConstructorType &
extract_constructor_type(TypeConstructor::AtomicType const &type);

bool is_composable(TypeConstructor const &, TypeConstructor const &);

TypeConstructor compose_type_constructors(TypeConstructor const &,
                                          TypeConstructor const &);

} // namespace Types
} // namespace Project

#endif

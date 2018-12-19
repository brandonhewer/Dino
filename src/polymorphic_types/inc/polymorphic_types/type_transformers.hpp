#ifndef __TYPE_TRANSFORMERS_HPP_
#define __TYPE_TRANSFORMERS_HPP_

#include "polymorphic_types/type_constructors.hpp"

namespace Project {
namespace Types {

TypeConstructor compose_type_constructors(TypeConstructor const &,
                                          TypeConstructor const &);
}
} // namespace Project

#endif

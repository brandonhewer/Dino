#ifndef __TYPE_EQUALITY_HPP_
#define __TYPE_EQUALITY_HPP_

#include "polymorphic_types/type_constructor.hpp"

namespace Project {
namespace Types {

TypeConstructor const &get_nested(TypeConstructor const &);

std::size_t const *get_identifier(TypeConstructor const &);

bool is_equal(TypeConstructor const &, TypeConstructor const &);

bool is_equal(FunctorTypeConstructor const &, FunctorTypeConstructor const &);

bool is_equal(TypeConstructor::Type const &, std::size_t);

bool is_equal(TypeConstructor::Type const &, MonoType);

bool is_equal(TypeConstructor::Type const &, FreeType);

bool is_equal(TypeConstructor::Type const &, TypeConstructor const &);

bool is_equal(TypeConstructor::Type const &, FunctorTypeConstructor const &);

} // namespace Types
} // namespace Project

#endif

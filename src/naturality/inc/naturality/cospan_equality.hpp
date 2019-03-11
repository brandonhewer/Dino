#ifndef __COSPAN_EQUALITY_HPP_
#define __COSPAN_EQUALITY_HPP_

#include "naturality/cospan.hpp"

namespace Project {
namespace Naturality {

CospanMorphism const &get_nested(CospanMorphism const &);

std::size_t const *get_identifier(CospanMorphism const &);

CospanMorphism::PairType const *get_pair(CospanMorphism const &);

bool is_equal(CospanMorphism const &, CospanMorphism const &);

bool is_equal(CospanMorphism::Type const &, CospanMorphism const &);

bool is_equal(CospanMorphism::Type const &, std::size_t);

bool is_equal(CospanMorphism::Type const &, EmptyType);

bool is_equal(CospanMorphism::Type const &, CospanMorphism::PairType const &);

} // namespace Naturality
} // namespace Project

#endif

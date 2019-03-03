#ifndef __COSPAN_EQUALITY_HPP_
#define __COSPAN_EQUALITY_HPP_

#include "naturality/cospan.hpp"

namespace Project {
namespace Naturality {

CospanMorphism const &get_nested(CospanMorphism const &);

std::size_t const *get_identifier(CospanMorphism const &);

bool is_equal(CospanMorphism const &, CospanMorphism const &);

bool is_equal(CospanMorphism::Type const &, CospanMorphism const &);

bool is_equal(CospanMorphism::Type const &, std::size_t);

bool is_equal(CospanMorphism::Type const &, EmptyType);

bool is_equal(CospanMorphism::Type const &,
              std::pair<std::size_t, std::size_t> const &);

} // namespace Naturality
} // namespace Project

#endif

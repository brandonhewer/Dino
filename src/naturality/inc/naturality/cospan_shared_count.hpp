#ifndef __COSPAN_MAXIMUM_HPP_
#define __COSPAN_MAXIMUM_HPP_

#include "naturality/cospan.hpp"

namespace Project {
namespace Naturality {

std::vector<std::pair<std::size_t, std::size_t>>
shared_count(std::vector<CospanMorphism> const &);

} // namespace Naturality
} // namespace Project

#endif

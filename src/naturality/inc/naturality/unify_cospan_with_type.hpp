#ifndef __UNIFY_COSPAN_WITH_TYPE_HPP_
#define __UNIFY_COSPAN_WITH_TYPE_HPP_

#include "naturality/cospan.hpp"
#include "naturality/natural_transformation.hpp"

namespace Project {
namespace Naturality {

std::vector<std::size_t>
unify_cospan_with_type(NaturalTransformation const &transformation,
                       CospanStructure &cospan);

} // namespace Naturality
} // namespace Project

#endif

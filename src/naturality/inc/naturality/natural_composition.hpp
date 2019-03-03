#ifndef __NATURAL_COMPOSITION_HPP_
#define __NATURAL_COMPOSITION_HPP_

#include "naturality/cospan.hpp"
#include "naturality/natural_transformation.hpp"

namespace Project {
namespace Naturality {

bool is_composable(NaturalTransformation const &,
                   NaturalTransformation const &);

NaturalTransformation compose_transformations(NaturalTransformation const &,
                                              NaturalTransformation const &);

} // namespace Naturality
} // namespace Project

#endif

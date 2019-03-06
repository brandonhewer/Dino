#ifndef __COSPAN_COMPOSITION_HPP_
#define __COSPAN_COMPOSITION_HPP_

#include "naturality/cospan.hpp"
#include "naturality/natural_transformation.hpp"

namespace Project {
namespace Naturality {

CospanStructure compose_cospans(CospanStructure const &,
                                CospanStructure const &,
                                NaturalTransformation const &, std::size_t);
}
} // namespace Project

#endif

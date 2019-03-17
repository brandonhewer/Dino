#ifndef __COSPAN_COMPOSITION_HPP_
#define __COSPAN_COMPOSITION_HPP_

#include "naturality/cospan.hpp"
#include "naturality/natural_transformation.hpp"
#include "polymorphic_types/unification.hpp"

namespace Project {
namespace Naturality {

struct CompositionResult {
  CospanStructure cospan;
  std::vector<std::size_t> value_count;
};

CompositionResult compose_cospans(CospanStructure const &,
                                  CospanStructure const &,
                                  NaturalTransformation const &,
                                  NaturalTransformation const &,
                                  Types::Unification const &, std::size_t);
} // namespace Naturality
} // namespace Project

#endif

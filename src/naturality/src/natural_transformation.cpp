#include "naturality/natural_transformation.hpp"
#include "polymorphic_types/type_transformers.hpp"

namespace Project {
namespace Naturality {

std::string to_string(NaturalTransformation const &transformation) {
  return to_string(transformation.domain, transformation.symbols) + " => " +
         to_string(transformation.codomain, transformation.symbols);
}

} // namespace Naturality
} // namespace Project
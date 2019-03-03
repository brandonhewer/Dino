#include "naturality/natural_transformation.hpp"
#include "polymorphic_types/type_to_string.hpp"

namespace Project {
namespace Naturality {

std::string to_string(NaturalTransformation const &transformation) {
  return to_string(transformation.domains.front(), transformation.symbols) +
         " => " +
         to_string(transformation.domains.back(), transformation.symbols);
}

} // namespace Naturality
} // namespace Project
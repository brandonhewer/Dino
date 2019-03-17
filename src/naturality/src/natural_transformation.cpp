#include "naturality/natural_transformation.hpp"
#include "polymorphic_types/type_to_string.hpp"

namespace Project {
namespace Naturality {

std::string to_string(NaturalTransformation const &transformation) {
  return to_string(transformation.domains.front(), transformation.symbols,
                   transformation.functor_symbols) +
         " => " +
         to_string(transformation.domains.back(), transformation.symbols,
                   transformation.functor_symbols);
}

std::string debug_string(NaturalTransformation const &transformation) {
  auto const &domains = transformation.domains;

  std::string result;
  for (auto type = domains.begin(); type < domains.end() - 1; ++type)
    result += to_string(*type, transformation.symbols,
                        transformation.functor_symbols) +
              " => ";
  return result + to_string(domains.back(), transformation.symbols,
                            transformation.functor_symbols);
}

} // namespace Naturality
} // namespace Project
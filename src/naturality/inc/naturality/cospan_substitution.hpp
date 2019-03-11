#ifndef __COSPAN_SUBSTITUTION_HPP_
#define __COSPAN_SUBSTITUTION_HPP_

#include "naturality/cospan.hpp"
#include "naturality/natural_transformation.hpp"
#include "polymorphic_types/unification.hpp"

namespace Project {
namespace Naturality {

struct CospanSubstitutions {
  std::vector<std::optional<CospanMorphism::Type>> values;
  std::size_t maximum;
};

using VariableSubstitution = std::vector<CospanSubstitutions>;

CospanMorphism cospan_substitution(
    VariableSubstitution &,
    std::vector<std::optional<Types::TypeConstructor::Type>> const &,
    CospanMorphism const &, Types::TypeConstructor const &);

VariableSubstitution create_empty_substitution(CospanStructure const &,
                                               NaturalTransformation const &);

} // namespace Naturality
} // namespace Project

#endif

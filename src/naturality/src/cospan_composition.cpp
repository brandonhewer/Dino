#include "naturality/cospan_composition.hpp"
#include "naturality/cospan_equality.hpp"
#include "naturality/cospan_substitution.hpp"
#include "naturality/cospan_zip.hpp"
#include "polymorphic_types/type_equality.hpp"

namespace {

using namespace Project::Naturality;
using namespace Project::Types;

template <typename StartCospanIt, typename StartTransformIt>
void add_substituted_domains(
    StartCospanIt start_cospan, StartTransformIt start_transform,
    std::size_t domains,
    std::vector<std::optional<TypeConstructor::Type>> const &unification,
    VariableSubstitution &substitutions) {
  for (auto i = 0u; i < domains; ++i)
    domains.emplace_back(cospan_substitution(substitutions, unification,
                                             *(start_cospan + i),
                                             *(start_transform + i)));
}

template <typename StartCospanIt, typename StartTransformIt>
std::vector<CospanMorphism> get_substituted_domains(
    StartCospanIt start_cospan, StartTransformIt start_transform,
    std::size_t domains,
    std::vector<std::optional<TypeConstructor::Type>> const &unification,
    VariableSubstitution &substitutions) {
  std::vector<CospanMorphism> domains;
  domains.reserve(domains);
  add_substituted_domains(start_cospan, start_transform, domains,
                          transformation, unification, substitutions);
  return std::move(domains);
}

CospanMorphism get_zipped_substitution(
    CospanMorphism const &left_morphism, CospanMorphism const &right_morphism,
    TypeConstructor const &left_type, TypeConstructor const &right_type,
    Unification const &unification, VariableSubstitution &left_substitution,
    VariableSubstitution &right_substitution) {
  auto const left = cospan_substitution(left_substitution, unification.left,
                                        left_morphism, left_type);
  auto const right = cospan_substitution(right_substitution, unification.right,
                                         right_morphism, right_type);
  return zip_cospan_morphisms(left, right);
}

} // namespace

namespace Project {
namespace Naturality {

CospanStructure compose_cospans(CospanStructure const &left_cospan,
                                CospanStructure const &right_cospan,
                                NaturalTransformation const &left_transform,
                                NaturalTransformation const &right_transform,
                                Types::Unification const &unification) {
  auto left_substitution =
      create_empty_substitution(left_cospan, left_transform);
  auto right_substitution =
      create_empty_substitution(right_cospan, right_transform);
  auto domains = get_substituted_domains(
      left_cospan.domains.begin(), left_transform.domains.begin(),
      left_cospan.domains.size() - 1, unification.left, left_substitution);

  domains.reserve(right_transform.domains.size());
  domains.emplace_back(get_zipped_substitution(
      left_cospan.domains.back(), right_cospan.domains.back(),
      left_transform.domains.back(), right_transform.domains.back(),
      unification, left_substitution, right_substitution));

  add_substituted_domains(right_cospan.domains.begin() + 1,
                          right_transform.domains.begin() + 1,
                          right_transform.domains.size() - 1, unification.right,
                          right_substitution);

  return {std::move(domains)};
}

} // namespace Naturality
} // namespace Project

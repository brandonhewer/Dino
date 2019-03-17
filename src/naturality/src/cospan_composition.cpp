#include "naturality/cospan_composition.hpp"
#include "naturality/cospan_equality.hpp"
#include "naturality/cospan_shared_count.hpp"
#include "naturality/cospan_substitution.hpp"
#include "naturality/cospan_zip.hpp"
#include "polymorphic_types/type_equality.hpp"

#include <iostream>

namespace {

using namespace Project::Naturality;
using namespace Project::Types;

template <typename StartCospanIt, typename StartTransformIt>
void add_substituted_domains(
    std::vector<CospanMorphism> &domains, StartCospanIt start_cospan,
    StartTransformIt start_transform, std::size_t number_of_domains,
    std::vector<std::optional<TypeConstructor::Type>> const &unification,
    VariableSubstitution &substitutions) {
  for (auto i = 0u; i < number_of_domains; ++i)
    domains.emplace_back(cospan_substitution(substitutions, unification,
                                             *(start_cospan + i),
                                             *(start_transform + i)));
}

template <typename StartCospanIt, typename StartTransformIt>
std::vector<CospanMorphism> get_substituted_domains(
    StartCospanIt start_cospan, StartTransformIt start_transform,
    std::size_t number_of_domains,
    std::vector<std::optional<TypeConstructor::Type>> const &unification,
    VariableSubstitution &substitutions) {
  std::vector<CospanMorphism> domains;
  domains.reserve(number_of_domains);
  add_substituted_domains(domains, start_cospan, start_transform,
                          number_of_domains, unification, substitutions);
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

std::vector<std::size_t>
maximum_counts(VariableSubstitution const &substitution) {
  std::vector<std::size_t> count(substitution[0].maximum.size(), 0);
  for (auto &&cospan_substitution : substitution) {
    auto &maximums = cospan_substitution.maximum;
    for (auto i = 0u; i < maximums.size(); ++i) {
      auto &current = count[i];
      current = std::max(current, maximums[i]);
    }
  }
  return std::move(count);
}

std::size_t get_max(std::vector<std::size_t> const &vec) {
  return *std::max_element(vec.begin(), vec.end());
}

} // namespace

namespace Project {
namespace Naturality {

CompositionResult compose_cospans(CospanStructure const &left_cospan,
                                  CospanStructure const &right_cospan,
                                  NaturalTransformation const &left_transform,
                                  NaturalTransformation const &right_transform,
                                  Types::Unification const &unification,
                                  std::size_t identifiers) {
  auto left_substitution =
      create_empty_substitution(left_cospan, left_transform, identifiers);

  auto domains = get_substituted_domains(
      left_cospan.domains.begin(), left_transform.domains.begin(),
      left_cospan.domains.size() - 1, unification.left, left_substitution);

  auto right_substitution = create_empty_substitution(
      right_cospan, right_transform, maximum_counts(left_substitution));

  domains.reserve(right_transform.domains.size());
  domains.emplace_back(CospanMorphism{});

  add_substituted_domains(domains, right_cospan.domains.begin() + 1,
                          right_transform.domains.begin() + 1,
                          right_transform.domains.size() - 1, unification.right,
                          right_substitution);

  domains[left_cospan.domains.size() - 1] = get_zipped_substitution(
      left_cospan.domains.back(), right_cospan.domains.front(),
      left_transform.domains.back(), right_transform.domains.front(),
      unification, left_substitution, right_substitution);

  auto min_max_identifiers = shared_count(domains);
  auto max_counts = maximum_counts(right_substitution);
  auto max_count = get_max(max_counts);
  return {{std::move(domains), std::move(min_max_identifiers), 0, max_count},
          std::move(max_counts)};
}

} // namespace Naturality
} // namespace Project

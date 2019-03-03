#include "naturality/natural_composition.hpp"
#include "naturality/cospan_unification.hpp"
#include "polymorphic_types/substitution.hpp"
#include "polymorphic_types/type_replacement.hpp"
#include "polymorphic_types/unification.hpp"

#include <optional>

namespace {

using namespace Project::Types;
using namespace Project::Naturality;

struct LRReplacements {
  Replacements left;
  Replacements right;
  std::size_t number;
};

Replacements shifted_identifiers(Substitution const &substitution,
                                 std::size_t &offset) {
  Replacements shift;
  shift.reserve(substitution.size());

  for (auto &&replacement : substitution) {
    if (!replacement.has_value())
      shift.emplace_back(offset++);
    else
      shift.emplace_back(std::nullopt);
  }
  return std::move(shift);
}

LRReplacements calculate_replacements(Unification &unification) {
  LRReplacements replacements;
  replacements.number = 0;
  replacements.left =
      shifted_identifiers(unification.left, replacements.number);
  replacements.right =
      shifted_identifiers(unification.right, replacements.number);
  return std::move(replacements);
}

void apply_replacements(
    std::vector<std::optional<TypeConstructor::Type>> &substitutions,
    Replacements const &new_substitutions, Replacements const &replacements) {
  for (auto i = 0u; i < substitutions.size(); ++i) {
    if (auto substitution = substitutions[i])
      replace_identifiers(*substitution, replacements);
    else
      substitutions[i] = new_substitutions[i];
  }
}

LRReplacements calculate_applied_replacements(Unification &unification) {
  auto replacements = calculate_replacements(unification);
  apply_replacements(unification.left, replacements.left, replacements.right);
  apply_replacements(unification.right, replacements.right, replacements.left);
  return std::move(replacements);
}

void add_new_identifiers(std::vector<std::string> &new_identifiers,
                         std::vector<std::string> const &identifiers,
                         Replacements const &replacements) {
  for (auto i = 0u; i < replacements.size(); ++i) {
    if (replacements[i].has_value())
      new_identifiers.emplace_back(identifiers[i]);
  }
}

std::vector<std::string>
get_new_identifiers(std::vector<std::string> const &left_identifiers,
                    std::vector<std::string> const &right_identifiers,
                    LRReplacements const &replacements) {
  std::vector<std::string> identifiers;
  identifiers.reserve(replacements.number);
  add_new_identifiers(identifiers, left_identifiers, replacements.left);
  add_new_identifiers(identifiers, right_identifiers, replacements.right);
  return std::move(identifiers);
}

std::vector<TypeConstructor>
apply_substitution_to(std::vector<TypeConstructor> const &domains,
                      Substitution const &substitution) {
  std::vector<TypeConstructor> substituted;
  substituted.reserve(domains.size());
  for (auto &&domain : domains)
    substituted.emplace_back(apply_substitution(domain, substitution));
  return std::move(substituted);
}

template <typename StartIt, typename EndIt>
void add_substituted_domains(StartIt start_iterator, EndIt end_iterator,
                             std::vector<TypeConstructor> &domains,
                             Substitution const &substitution) {
  for (auto it = start_iterator; it < end_iterator; ++it)
    domains.emplace_back(apply_substitution(*it, substitution));
}

NaturalTransformation compose_with(NaturalTransformation const &left,
                                   NaturalTransformation const &right,
                                   Unification &unification) {
  auto const replacements = calculate_applied_replacements(unification);
  auto substituted = apply_substitution_to(left.domains, unification.left);

  substituted.reserve(substituted.size() + right.domains.size() - 1);

  add_substituted_domains(right.domains.begin() + 1, right.domains.end(),
                          substituted, unification.right);

  auto symbols = get_new_identifiers(left.symbols, right.symbols, replacements);
  return NaturalTransformation{std::move(substituted), std::move(symbols)};
}

} // namespace

namespace Project {
namespace Naturality {

bool is_composable(NaturalTransformation const &left,
                   NaturalTransformation const &right) {
  return calculate_unification(left.domains.back(), right.domains.front(),
                               left.symbols.size(), right.symbols.size())
      .has_value();
}

NaturalTransformation
compose_transformations(NaturalTransformation const &left,
                        NaturalTransformation const &right) {
  auto unification =
      calculate_unification(left.domains.back(), right.domains.front(),
                            left.symbols.size(), right.symbols.size());

  if (!unification)
    throw std::runtime_error("Failed to compose");
  return compose_with(left, right, *unification);
}

} // namespace Naturality
} // namespace Project

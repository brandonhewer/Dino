#include "naturality/natural_composition.hpp"
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

void add_shifted_identifiers(Replacements &, std::size_t &,
                             TypeConstructor::Type const &);

void shift_constructor_identifiers(
    Replacements &replacements, std::size_t &offset,
    TypeConstructor::ConstructorType const &constructor) {
  for (auto &&type : constructor)
    add_shifted_identifiers(replacements, offset, type.type);
}

void shift_constructor_identifiers(Replacements &replacements,
                                   std::size_t &offset,
                                   TypeConstructor const &constructor) {
  shift_constructor_identifiers(replacements, offset, constructor.type);
}

void shift_functor_identifiers(Replacements &replacements, std::size_t &offset,
                               FunctorTypeConstructor const &functor) {
  if (!replacements[functor.identifier].has_value())
    replacements[functor.identifier] = offset++;
  shift_constructor_identifiers(replacements, offset, functor.type);
}

struct AddShiftedIdentifiers {

  void operator()(Replacements &replacements, std::size_t &offset,
                  std::size_t identifier) const {
    if (!replacements[identifier].has_value())
      replacements[identifier] = offset++;
  }

  void operator()(Replacements &replacements, std::size_t &offset,
                  TypeConstructor const &constructor) const {
    shift_constructor_identifiers(replacements, offset, constructor);
  }

  void operator()(Replacements &replacements, std::size_t &offset,
                  FunctorTypeConstructor const &functor) const {
    shift_functor_identifiers(replacements, offset, functor);
  }

  template <typename T>
  void operator()(Replacements &, std::size_t &, T const &) const {}

} _add_shifted_identifiers;

void add_shifted_identifiers(Replacements &replacements, std::size_t &offset,
                             TypeConstructor::Type const &type) {
  std::visit(std::bind(_add_shifted_identifiers, std::ref(replacements),
                       std::ref(offset), std::placeholders::_1),
             type);
}

void shift_used_in_identifiers(Substitution const &substitution,
                               Replacements &shifted, std::size_t &offset) {
  for (auto &&replacement : substitution) {
    if (replacement.has_value())
      add_shifted_identifiers(shifted, offset, *replacement);
  }
}

void shift_identifiers(Substitution const &substitution, Replacements &shifted,
                       std::size_t &offset) {
  for (auto i = 0u; i < substitution.size(); ++i) {
    if (!substitution[i].has_value())
      shifted[i] = offset++;
  }
}

void shift_identifiers(Substitution const &substitution,
                       Substitution const &used_in, Replacements &shifted,
                       std::size_t &offset) {
  shift_identifiers(substitution, shifted, offset);
  shift_used_in_identifiers(used_in, shifted, offset);
}

LRReplacements calculate_replacements(Unification &unification) {
  LRReplacements replacements;
  replacements.number = 0;
  replacements.left = Replacements(unification.left.size(), std::nullopt);
  replacements.right = Replacements(unification.right.size(), std::nullopt);

  shift_identifiers(unification.left, unification.right, replacements.left,
                    replacements.number);
  shift_identifiers(unification.right, unification.left, replacements.right,
                    replacements.number);

  return std::move(replacements);
}

void apply_replacements(
    std::vector<std::optional<TypeConstructor::Type>> &substitutions,
    Replacements const &new_substitutions, Replacements const &replacements) {
  for (auto i = 0u; i < substitutions.size(); ++i) {
    if (auto substitution = substitutions[i])
      replace_identifiers(*substitution, replacements);
    else if (auto substitution = new_substitutions[i])
      substitutions[i] = substitution;
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
    throw std::runtime_error("Failed to compose types");
  return compose_with(left, right, *unification);
}

} // namespace Naturality
} // namespace Project

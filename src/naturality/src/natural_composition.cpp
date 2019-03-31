#include "naturality/natural_composition.hpp"
#include "polymorphic_types/substitution.hpp"
#include "polymorphic_types/type_replacement.hpp"

#include <optional>

namespace {

using namespace Project::Types;
using namespace Project::Naturality;

struct LRReplacements {
  TypeReplacements left;
  TypeReplacements right;
  std::size_t number;
};

struct UsedFunctorIdentifiers {
  std::vector<bool> left;
  std::vector<bool> right;
};

void add_shifted_identifiers(TypeReplacements &, std::size_t &,
                             TypeConstructor::Type const &);

void shift_constructor_identifiers(
    TypeReplacements &replacements, std::size_t &offset,
    TypeConstructor::ConstructorType const &constructor) {
  for (auto &&type : constructor)
    add_shifted_identifiers(replacements, offset, type.type);
}

void shift_constructor_identifiers(TypeReplacements &replacements,
                                   std::size_t &offset,
                                   TypeConstructor const &constructor) {
  shift_constructor_identifiers(replacements, offset, constructor.type);
}

void shift_functor_identifiers(TypeReplacements &replacements,
                               std::size_t &offset,
                               FunctorTypeConstructor const &functor) {
  shift_constructor_identifiers(replacements, offset, functor.type);
}

struct AddShiftedIdentifiers {

  void operator()(TypeReplacements &replacements, std::size_t &offset,
                  std::size_t identifier) const {
    if (!replacements[identifier].has_value())
      replacements[identifier] = offset++;
  }

  void operator()(TypeReplacements &replacements, std::size_t &offset,
                  TypeConstructor const &constructor) const {
    shift_constructor_identifiers(replacements, offset, constructor);
  }

  void operator()(TypeReplacements &replacements, std::size_t &offset,
                  FunctorTypeConstructor const &functor) const {
    shift_functor_identifiers(replacements, offset, functor);
  }

  template <typename T>
  void operator()(TypeReplacements &, std::size_t &, T const &) const {}

} _add_shifted_identifiers;

void add_shifted_identifiers(TypeReplacements &replacements,
                             std::size_t &offset,
                             TypeConstructor::Type const &type) {
  std::visit(std::bind(_add_shifted_identifiers, std::ref(replacements),
                       std::ref(offset), std::placeholders::_1),
             type);
}

void shift_used_in_identifiers(Substitution const &substitution,
                               TypeReplacements &shifted, std::size_t &offset) {
  for (auto &&replacement : substitution) {
    if (replacement.has_value())
      add_shifted_identifiers(shifted, offset, *replacement);
  }
}

void shift_identifiers(Substitution const &substitution,
                       TypeReplacements &shifted, std::size_t &offset) {
  for (auto i = 0u; i < substitution.size(); ++i) {
    if (!substitution[i].has_value())
      shifted[i] = offset++;
  }
}

void shift_identifiers(Substitution const &substitution,
                       Substitution const &used_in, TypeReplacements &shifted,
                       std::size_t &offset) {
  shift_identifiers(substitution, shifted, offset);
  shift_used_in_identifiers(used_in, shifted, offset);
}

LRReplacements calculate_replacements(Unification const &unification) {
  LRReplacements replacements;
  replacements.number = 0;
  replacements.left = TypeReplacements(unification.left.size(), std::nullopt);
  replacements.right = TypeReplacements(unification.right.size(), std::nullopt);

  shift_identifiers(unification.left, unification.right, replacements.left,
                    replacements.number);
  shift_identifiers(unification.right, unification.left, replacements.right,
                    replacements.number);

  return std::move(replacements);
}

void apply_replacements(
    std::vector<std::optional<TypeConstructor::Type>> &substitutions,
    TypeReplacements const &new_substitutions,
    TypeReplacements const &replacements) {
  for (auto i = 0u; i < substitutions.size(); ++i) {
    if (auto substitution = substitutions[i])
      replace_identifiers(*substitution, replacements);
    else if (auto substitution = new_substitutions[i])
      substitutions[i] = substitution;
  }
}

void add_used_identifiers(
    std::vector<std::optional<std::size_t>> const &substitutions,
    std::vector<bool> &substituted, std::vector<bool> &used_in) {
  for (auto i = 0u; i < substitutions.size(); ++i) {
    if (auto const identifier = substitutions[i])
      used_in[*identifier] = true;
    else
      substituted[i] = true;
  }
}

UsedFunctorIdentifiers
get_used_identifiers(FunctorSubstitution const &left_substitutions,
                     FunctorSubstitution const &right_substitutions) {
  UsedFunctorIdentifiers used_identifiers{
      std::vector<bool>(left_substitutions.size() + 1, false),
      std::vector<bool>(right_substitutions.size() + 1, false)};
  add_used_identifiers(left_substitutions, used_identifiers.left,
                       used_identifiers.right);
  add_used_identifiers(right_substitutions, used_identifiers.right,
                       used_identifiers.left);
  return std::move(used_identifiers);
}

void add_shifted_functor_identifiers(
    std::vector<bool> const &used_identifiers,
    std::vector<std::optional<std::size_t>> &substitutions,
    std::size_t &identifier) {
  for (auto i = 0u; i < used_identifiers.size(); ++i) {
    if (used_identifiers[i])
      substitutions[i] = identifier++;
  }
}

UsedFunctorIdentifiers shift_functor_identifiers(Unification &unification) {
  auto const used_functor =
      get_used_identifiers(unification.functor_left, unification.functor_right);

  std::size_t count = 0;
  add_shifted_functor_identifiers(used_functor.left, unification.functor_left,
                                  count);
  add_shifted_functor_identifiers(used_functor.right, unification.functor_right,
                                  count);
  return std::move(used_functor);
}

LRReplacements calculate_applied_replacements(Unification &unification) {
  auto replacements = calculate_replacements(unification);
  apply_replacements(unification.left, replacements.left, replacements.right);
  apply_replacements(unification.right, replacements.right, replacements.left);
  return std::move(replacements);
}

void add_new_identifiers(std::vector<std::string> &new_identifiers,
                         std::vector<std::string> const &identifiers,
                         TypeReplacements const &replacements) {
  for (auto i = 0u; i < identifiers.size(); ++i) {
    if (replacements[i].has_value())
      new_identifiers.emplace_back(identifiers[i]);
  }
}

void add_new_identifiers(std::vector<std::string> &new_identifiers,
                         std::vector<std::string> const &identifiers,
                         std::vector<bool> const &used) {
  for (auto i = 0u; i < identifiers.size(); ++i) {
    if (used[i])
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

std::vector<std::string>
get_new_identifiers(std::vector<std::string> const &left_identifiers,
                    std::vector<std::string> const &right_identifiers,
                    UsedFunctorIdentifiers const &used_functor) {
  std::vector<std::string> identifiers;
  identifiers.reserve(used_functor.left.size() + used_functor.right.size());
  add_new_identifiers(identifiers, left_identifiers, used_functor.left);
  add_new_identifiers(identifiers, right_identifiers, used_functor.right);
  return std::move(identifiers);
}

std::vector<TypeConstructor>
apply_substitution_to(std::vector<TypeConstructor> const &domains,
                      Substitution const &substitution,
                      FunctorSubstitution const &functor_substitution) {
  std::vector<TypeConstructor> substituted;
  substituted.reserve(domains.size());
  for (auto &&domain : domains)
    substituted.emplace_back(
        apply_substitution(domain, substitution, functor_substitution));
  return std::move(substituted);
}

template <typename StartIt, typename EndIt>
void add_substituted_domains(StartIt start_iterator, EndIt end_iterator,
                             std::vector<TypeConstructor> &domains,
                             Substitution const &substitution,
                             FunctorSubstitution const &functor_substitution) {
  for (auto it = start_iterator; it < end_iterator; ++it)
    domains.emplace_back(
        apply_substitution(*it, substitution, functor_substitution));
}

} // namespace

namespace Project {
namespace Naturality {

bool is_composable(NaturalTransformation const &left,
                   NaturalTransformation const &right) {
  return calculate_unification(left.domains.back(), right.domains.front(),
                               left.symbols.size(), right.symbols.size(),
                               left.functor_symbols.size(),
                               right.functor_symbols.size())
      .has_value();
}

NaturalTransformation
compose_transformations(NaturalTransformation const &left,
                        NaturalTransformation const &right,
                        Types::Unification &unification) {
  auto const replacements = calculate_applied_replacements(unification);
  auto const used_functor = shift_functor_identifiers(unification);

  auto substituted = apply_substitution_to(left.domains, unification.left,
                                           unification.functor_left);

  substituted.reserve(substituted.size() + right.domains.size() - 1);

  add_substituted_domains(right.domains.begin() + 1, right.domains.end(),
                          substituted, unification.right,
                          unification.functor_right);

  auto symbols = get_new_identifiers(left.symbols, right.symbols, replacements);
  auto functor_symbols = get_new_identifiers(
      left.functor_symbols, right.functor_symbols, used_functor);
  return NaturalTransformation{std::move(substituted), std::move(symbols),
                               std::move(functor_symbols)};
}

NaturalTransformation
compose_transformations(NaturalTransformation const &left,
                        NaturalTransformation const &right) {
  auto unification = calculate_unification(
      left.domains.back(), right.domains.front(), left.symbols.size(),
      right.symbols.size(), left.functor_symbols.size(),
      right.functor_symbols.size());

  if (!unification)
    throw std::runtime_error("Failed to compose types");
  return compose_transformations(left, right, *unification);
}

} // namespace Naturality
} // namespace Project

#include "polymorphic_types/type_replacement.hpp"

namespace {

using namespace Project::Types;

void replace_all_identifiers(TypeReplacements const &, TypeConstructor::Type &);

void replace_all_identifiers(TypeReplacements const &replacements,
                             TypeConstructor::ConstructorType &constructor) {
  for (auto &&type : constructor)
    replace_all_identifiers(replacements, type.type);
}

void replace_all_identifiers(TypeReplacements const &replacements,
                             FunctorTypeConstructor &functor) {
  replace_all_identifiers(replacements, functor.type);
}

struct ReplaceIdentifiers {

  void operator()(TypeReplacements const &replacements,
                  std::size_t &identifier) const {
    if (auto replaced = replacements[identifier])
      identifier = *replaced;
  }

  void operator()(TypeReplacements const &replacements,
                  TypeConstructor &constructor) const {
    replace_all_identifiers(replacements, constructor.type);
  }

  void operator()(TypeReplacements const &replacements,
                  FunctorTypeConstructor &functor) const {
    replace_all_identifiers(replacements, functor);
  }

  template <typename T> void operator()(TypeReplacements const &, T &) const {}

} _replace_all_identifiers;

void replace_all_identifiers(TypeReplacements const &replacements,
                             TypeConstructor::Type &type) {
  std::visit(std::bind(_replace_all_identifiers, std::cref(replacements),
                       std::placeholders::_1),
             type);
}

} // namespace

namespace Project {
namespace Types {

TypeConstructor::Type &
replace_identifiers(TypeConstructor::Type &type,
                    TypeReplacements const &replacements) {
  replace_all_identifiers(replacements, type);
  return type;
}

TypeConstructor &replace_identifiers(TypeConstructor &constructor,
                                     TypeReplacements const &replacements) {
  replace_all_identifiers(replacements, constructor.type);
  return constructor;
}

} // namespace Types
} // namespace Project

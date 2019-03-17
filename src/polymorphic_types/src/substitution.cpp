#include "polymorphic_types/substitution.hpp"

#include "polymorphic_types/type_to_string.hpp"

namespace {

using namespace Project::Types;

TypeConstructor::Type substitute_type(Substitution const &,
                                      FunctorSubstitution const &,
                                      TypeConstructor::Type const &);

TypeConstructor::Type substitute_identifier(Substitution const &substitution,
                                            std::size_t identifier) {
  if (substitution.size() <= identifier)
    return identifier;

  if (auto const &type = substitution[identifier])
    return *type;
  return identifier;
}

TypeConstructor::ConstructorType
substitute_constructor(Substitution const &substitution,
                       FunctorSubstitution const &functor_substitution,
                       TypeConstructor::ConstructorType const &constructor) {
  TypeConstructor::ConstructorType substituted;
  substituted.reserve(constructor.size());
  for (auto &&type : constructor)
    substituted.emplace_back(TypeConstructor::AtomicType{
        substitute_type(substitution, functor_substitution, type.type),
        type.variance});
  return std::move(substituted);
}

TypeConstructor::Type
substitute_functor(Substitution const &substitution,
                   FunctorSubstitution const &functor_substitution,
                   FunctorTypeConstructor const &functor) {
  return FunctorTypeConstructor{
      substitute_constructor(substitution, functor_substitution, functor.type),
      functor_substitution[functor.identifier].value_or(functor.identifier)};
}

TypeConstructor
substitute_constructor(Substitution const &substitution,
                       FunctorSubstitution const &functor_substitution,
                       TypeConstructor const &constructor) {
  return {substitute_constructor(substitution, functor_substitution,
                                 constructor.type)};
}

struct SubstituteType {

  TypeConstructor::Type operator()(Substitution const &substitution,
                                   FunctorSubstitution const &,
                                   std::size_t identifier) const {
    return substitute_identifier(substitution, identifier);
  }

  TypeConstructor::Type
  operator()(Substitution const &substitution,
             FunctorSubstitution const &functor_substitution,
             FunctorTypeConstructor const &functor) const {
    return substitute_functor(substitution, functor_substitution, functor);
  }

  TypeConstructor::Type
  operator()(Substitution const &substitution,
             FunctorSubstitution const &functor_substitution,
             TypeConstructor const &constructor) const {
    return substitute_constructor(substitution, functor_substitution,
                                  constructor);
  }

  template <typename T>
  TypeConstructor::Type operator()(Substitution const &,
                                   FunctorSubstitution const &,
                                   T const &type) const {
    return type;
  }

} _substitute_type;

TypeConstructor::Type
substitute_type(Substitution const &substitution,
                FunctorSubstitution const &functor_substitution,
                TypeConstructor::Type const &type) {
  return std::visit(std::bind(_substitute_type, std::cref(substitution),
                              std::cref(functor_substitution),
                              std::placeholders::_1),
                    type);
}

} // namespace

namespace Project {
namespace Types {

TypeConstructor
apply_substitution(TypeConstructor const &type,
                   Substitution const &substitution,
                   FunctorSubstitution const &functor_substitution) {
  return substitute_constructor(substitution, functor_substitution, type);
}

} // namespace Types
} // namespace Project

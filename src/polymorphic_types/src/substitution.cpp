#include "polymorphic_types/substitution.hpp"

#include "polymorphic_types/type_to_string.hpp"

namespace {

using namespace Project::Types;

TypeConstructor::Type substitute_type(Substitution const &,
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
                       TypeConstructor::ConstructorType const &constructor) {
  TypeConstructor::ConstructorType substituted;
  substituted.reserve(constructor.size());
  for (auto &&type : constructor)
    substituted.emplace_back(TypeConstructor::AtomicType{
        substitute_type(substitution, type.type), type.variance});
  return std::move(substituted);
}

TypeConstructor::Type
substitute_in_functor(Substitution const &substitution,
                      TypeConstructor::ConstructorType const &types,
                      std::size_t identifier) {
  return FunctorTypeConstructor{substitute_constructor(substitution, types),
                                identifier};
}

TypeConstructor::Type
substitute_in_functor(Substitution const &substitution,
                      FunctorTypeConstructor const &functor) {
  return substitute_in_functor(substitution, functor.type, functor.identifier);
}

TypeConstructor::Type
substitute_in_functor(Substitution const &substitution,
                      TypeConstructor::Type const &type,
                      FunctorTypeConstructor const &functor) {
  if (auto const identifier = std::get_if<std::size_t>(&type))
    return substitute_in_functor(substitution, functor.type, *identifier);
  return type;
}

TypeConstructor::Type
substitute_functor(Substitution const &substitution,
                   FunctorTypeConstructor const &functor) {
  if (substitution.size() <= functor.identifier)
    return substitute_in_functor(substitution, functor);

  if (auto const &type = substitution[functor.identifier])
    return substitute_in_functor(substitution, *type, functor);
  return substitute_in_functor(substitution, functor);
}

TypeConstructor substitute_constructor(Substitution const &substitution,
                                       TypeConstructor const &constructor) {
  return {substitute_constructor(substitution, constructor.type)};
}

struct SubstituteType {

  TypeConstructor::Type operator()(Substitution const &substitution,
                                   std::size_t identifier) const {
    return substitute_identifier(substitution, identifier);
  }

  TypeConstructor::Type
  operator()(Substitution const &substitution,
             FunctorTypeConstructor const &functor) const {
    return substitute_functor(substitution, functor);
  }

  TypeConstructor::Type operator()(Substitution const &substitution,
                                   TypeConstructor const &constructor) const {
    return substitute_constructor(substitution, constructor);
  }

  template <typename T>
  TypeConstructor::Type operator()(Substitution const &substitution,
                                   T const &type) const {
    return type;
  }

} _substitute_type;

TypeConstructor::Type substitute_type(Substitution const &substitution,
                                      TypeConstructor::Type const &type) {
  return std::visit(std::bind(_substitute_type, std::cref(substitution),
                              std::placeholders::_1),
                    type);
}

} // namespace

namespace Project {
namespace Types {

TypeConstructor apply_substitution(TypeConstructor const &type,
                                   Substitution const &substitution) {
  return substitute_constructor(substitution, type);
}

} // namespace Types
} // namespace Project

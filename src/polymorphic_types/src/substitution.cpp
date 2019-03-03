#include "polymorphic_types/substitution.hpp"

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

TypeConstructor::Type
substitute_functor(Substitution const &substitution,
                   FunctorTypeConstructor const &functor) {
  return substitute_identifier(substitution, functor.identifier);
}

TypeConstructor substitute_constructor(Substitution const &substitution,
                                       TypeConstructor const &constructor) {
  TypeConstructor substituted;
  substituted.type.reserve(constructor.type.size());
  for (auto &&type : constructor.type)
    substituted.type.emplace_back(TypeConstructor::AtomicType{
        substitute_type(substitution, type.type), type.variance});
  return std::move(substituted);
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

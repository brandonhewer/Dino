#include "polymorphic_types/unification.hpp"
#include "polymorphic_types/type_equality.hpp"

namespace {

using namespace Project::Types;

bool compute_unification(TypeConstructor::ConstructorType const &,
                         TypeConstructor::ConstructorType const &,
                         Unification &);

template <typename T>
bool unify_identifier(
    std::vector<std::optional<TypeConstructor::Type>> &unification,
    std::size_t identifier, T const &unifier) {
  if (auto const &current = unification[identifier])
    return is_equal(*current, unifier);
  unification[identifier] = unifier;
  return true;
}

bool unify_functor(Unification &unification, FunctorTypeConstructor const &left,
                   FunctorTypeConstructor const &right) {
  if (!compute_unification(left.type, right.type, unification))
    return false;
  return unify_identifier(unification.left, left.identifier, right.identifier);
}

bool unify_functor(Unification &unification,
                   FunctorTypeConstructor const &functor,
                   TypeConstructor const &constructor) {
  auto const &nested = get_nested(constructor);

  if (auto const &current = unification.left[functor.identifier])
    return is_equal(*current, nested);

  if (!compute_unification(functor.type, nested.type, unification))
    return false;

  unification.left[functor.identifier] = nested;
  return true;
}

bool unify_functor(Unification &unification, TypeConstructor const &constructor,
                   FunctorTypeConstructor const &functor) {
  auto const &nested = get_nested(constructor);

  if (auto const &current = unification.right[functor.identifier])
    return is_equal(*current, nested);

  if (!compute_unification(nested.type, functor.type, unification))
    return false;

  unification.right[functor.identifier] = nested;
  return true;
}

bool unify_constructors(Unification &unification, TypeConstructor const &left,
                        TypeConstructor const &right) {
  return compute_unification(get_nested(left).type, get_nested(right).type,
                             unification);
}

struct ComputeUnification {

  bool operator()(Unification &unification, std::size_t left,
                  std::size_t right) const {
    return unify_identifier(unification.right, right, left);
  }

  template <typename T>
  bool operator()(Unification &unification, std::size_t identifier,
                  T const &type) const {
    return unify_identifier(unification.left, identifier, type);
  }

  template <typename T>
  bool operator()(Unification &unification, T const &type,
                  std::size_t identifier) const {
    return unify_identifier(unification.right, identifier, type);
  }

  bool operator()(Unification &unification, FunctorTypeConstructor const &left,
                  FunctorTypeConstructor const &right) const {
    return unify_functor(unification, left, right);
  }

  bool operator()(Unification &unification,
                  FunctorTypeConstructor const &functor,
                  TypeConstructor const &constructor) const {
    return unify_functor(unification, functor, constructor);
  }

  bool operator()(Unification &unification, TypeConstructor const &constructor,
                  FunctorTypeConstructor const &functor) const {
    return unify_functor(unification, constructor, functor);
  }

  bool operator()(Unification &unification, TypeConstructor const &left,
                  TypeConstructor const &right) const {
    return compute_unification(left.type, right.type, unification);
  }

  bool operator()(Unification &, MonoType left, MonoType right) const {
    return left == right;
  }

  bool operator()(Unification &, FreeType, FreeType) const { return true; }

  template <typename T, typename U>
  bool operator()(Unification &, T const &, U const &) const {
    return false;
  }

} _compute_unification;

template <typename F>
bool compute_unification(TypeConstructor::AtomicType const &left,
                         TypeConstructor::AtomicType const &right,
                         F const &unify) {
  return left.variance == right.variance &&
         std::visit(unify, left.type, right.type);
}

template <typename F>
bool compute_constructor_unification(TypeConstructor const &left,
                                     TypeConstructor::Type const &right,
                                     F const &unify) {
  return std::visit(std::bind(unify, std::cref(left), std::placeholders::_1),
                    right);
}

template <typename F>
bool compute_constructor_unification(TypeConstructor::Type const &left,
                                     TypeConstructor const &right,
                                     F const &unify) {
  return std::visit(std::bind(unify, std::placeholders::_1, std::cref(right)),
                    left);
}

TypeConstructor create_tail(TypeConstructor::ConstructorType const &constructor,
                            std::size_t from) {
  return {TypeConstructor::ConstructorType(constructor.begin() + from,
                                           constructor.end())};
}

template <typename F>
bool compute_unification_greater_left(
    TypeConstructor::ConstructorType const &left,
    TypeConstructor::ConstructorType const &right, F const &unify) {
  for (auto i = 0u; i < right.size() - 1; ++i) {
    if (!compute_unification(left[i], right[i], unify))
      return false;
  }
  return compute_constructor_unification(create_tail(left, right.size() - 1),
                                         right.back().type, unify);
}

template <typename F>
bool compute_unification_greater_right(
    TypeConstructor::ConstructorType const &left,
    TypeConstructor::ConstructorType const &right, F const &unify) {
  for (auto i = 0u; i < left.size() - 1; ++i) {
    if (!compute_unification(left[i], right[i], unify))
      return false;
  }
  return compute_constructor_unification(
      left.back().type, create_tail(right, left.size() - 1), unify);
}

template <typename F>
bool compute_unification_equal(TypeConstructor::ConstructorType const &left,
                               TypeConstructor::ConstructorType const &right,
                               F const &unify) {
  for (auto i = 0u; i < left.size(); ++i) {
    if (!compute_unification(left[i], right[i], unify))
      return false;
  }
  return true;
}

bool compute_unification(TypeConstructor::ConstructorType const &left,
                         TypeConstructor::ConstructorType const &right,
                         Unification &unification) {
  auto const unify = std::bind(_compute_unification, std::ref(unification),
                               std::placeholders::_1, std::placeholders::_2);

  if (left.size() > right.size())
    return compute_unification_greater_left(left, right, unify);
  else if (right.size() > left.size())
    return compute_unification_greater_right(left, right, unify);
  return compute_unification_equal(left, right, unify);
}

} // namespace

namespace Project {
namespace Types {

std::optional<Unification> calculate_unification(TypeConstructor const &left,
                                                 TypeConstructor const &right,
                                                 std::size_t left_symbols,
                                                 std::size_t right_symbols) {
  Unification unification{std::vector<std::optional<TypeConstructor::Type>>(
                              left_symbols, std::nullopt),
                          std::vector<std::optional<TypeConstructor::Type>>(
                              right_symbols, std::nullopt)};
  return unify_constructors(unification, left, right)
             ? std::move(unification)
             : std::optional<Unification>(std::nullopt);
}

} // namespace Types
} // namespace Project

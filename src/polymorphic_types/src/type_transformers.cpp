#include "polymorphic_types/type_transformers.hpp"

#include <functional>
#include <iostream>
#include <numeric>

namespace {

using namespace Project::Types;

bool is_valid_composition(TypeConstructor const &, TypeConstructor const &);

struct IsValidComposition {

  bool operator()(FreeType const &, FreeType const &) const { return true; }

  bool operator()(MonoType const &lhs, MonoType const &rhs) const {
    return lhs == rhs;
  }

  bool operator()(std::size_t, std::size_t) const { return true; }

  bool operator()(TypeConstructor const &lhs,
                  TypeConstructor const &rhs) const {
    return is_valid_composition(lhs, rhs);
  }

  bool operator()(TypeConstructor const &, std::size_t) const { return true; }

  template <typename T, typename U>
  bool operator()(T const &lhs, U const &rhs) const {
    return false;
  }

} _is_valid_composition;

bool is_valid_composition(TypeConstructor const &lhs,
                          TypeConstructor const &rhs) {
  for (auto i = 0u; i < lhs.type.size(); ++i) {
    auto const &lhs_type = lhs.type[i];
    auto const &rhs_type = rhs.type[i];
    if (lhs_type.variance != rhs_type.variance ||
        !std::visit(_is_valid_composition, lhs_type.type, rhs_type.type))
      return false;
  }
  return true;
}

void throw_if_invalid_composition(TypeConstructor::AtomicType const &lhs,
                                  TypeConstructor::AtomicType const &rhs) {
  if (lhs.variance == rhs.variance)
    throw CompositionError(lhs.variance, rhs.variance);
  if (!std::visit(_is_valid_composition, lhs.type, rhs.type))
    throw TypeMismatchError(lhs.type, rhs.type);
}

TypeConstructor::ConstructorType
compose_constructors(TypeConstructor::ConstructorType const &lhs,
                     TypeConstructor::ConstructorType const &rhs) {
  if (lhs.empty())
    return rhs;
  else if (rhs.empty())
    return lhs;

  throw_if_invalid_composition(lhs.back(), rhs.front());

  TypeConstructor::ConstructorType composed;
  composed.insert(composed.end(), lhs.begin(), lhs.end() - 1);
  composed.insert(composed.end(), rhs.begin() + 1, rhs.end());
  return std::move(composed);
}

struct ExtractConstructorType {

  TypeConstructor::ConstructorType &
  operator()(TypeConstructor &constructor) const {
    return constructor.type;
  }

  template <typename T>
  TypeConstructor::ConstructorType &operator()(T &) const {
    throw InvalidTypeAccessError();
  }
} _extract_constructor_type;

std::string mono_type_to_string(MonoType const &type) {
  switch (type) {
  case MonoType::CHAR:
    return "Char";
  case MonoType::INT:
    return "Int";
  case MonoType::FLOAT:
    return "Float";
  }
}

std::string variance_to_string(Variance const &variance) {
  switch (variance) {
  case Variance::COVARIANCE:
    return "";
  case Variance::CONTRAVARIANCE:
    return "-";
  case Variance::BIVARIANCE:
    return "+-";
  case Variance::INVARIANCE:
    return "!";
  }
}

std::string type_to_string(TypeConstructor::Type const &,
                           std::vector<std::string> const &);

std::string
functor_variable_to_string(TypeConstructor::AtomicType const &type,
                           std::vector<std::string> const &symbols) {
  return type_to_string(type.type, symbols) + variance_to_string(type.variance);
}

std::string functor_to_string(FunctorTypeConstructor const &functor,
                              std::vector<std::string> const &symbols) {
  auto const add_type = [&symbols](auto const &acc, auto const &type) {
    return acc + " " + functor_variable_to_string(type, symbols);
  };
  return std::accumulate(functor.type.begin(), functor.type.end(),
                         symbols[functor.identifier], add_type);
}

template <typename StartIt, typename EndIt>
std::string types_to_string(StartIt start_it, EndIt end_it, Variance variance,
                            std::vector<std::string> const &symbols) {
  std::string type_string;
  for (auto type_it = start_it; type_it < end_it; ++type_it) {
    auto const &type = *type_it;
    auto const delimiter = variance == Variance::CONTRAVARIANCE ? " -> " : " ";
    type_string += delimiter + type_to_string(type.type, symbols);
    variance = type.variance;
  }
  return std::move(type_string);
}

std::string constructor_to_string(TypeConstructor const &constructor,
                                  std::vector<std::string> const &symbols) {
  if (constructor.type.empty())
    return "";

  auto const &first_type = constructor.type.front();
  return type_to_string(first_type.type, symbols) +
         types_to_string(constructor.type.begin() + 1, constructor.type.end(),
                         first_type.variance, symbols);
}

struct TypeToString {

  std::string operator()(std::vector<std::string> const &symbols,
                         std::size_t identifier) const {
    return symbols[identifier];
  }

  std::string operator()(std::vector<std::string> const &,
                         FreeType const &) const {
    return "*";
  }

  std::string operator()(std::vector<std::string> const &,
                         MonoType const &type) const {
    return mono_type_to_string(type);
  }

  std::string operator()(std::vector<std::string> const &symbols,
                         FunctorTypeConstructor const &functor) const {
    return functor_to_string(functor, symbols);
  }

  std::string operator()(std::vector<std::string> const &symbols,
                         TypeConstructor const &constructor) const {
    return "(" + constructor_to_string(constructor, symbols) + ")";
  }

} _type_to_string;

std::string type_to_string(TypeConstructor::Type const &type,
                           std::vector<std::string> const &symbols) {
  return std::visit(
      std::bind(_type_to_string, std::cref(symbols), std::placeholders::_1),
      type);
}

} // namespace

namespace Project {
namespace Types {

TypeConstructor::ConstructorType &
extract_constructor_type(TypeConstructor::AtomicType const &type) {
  return std::visit(_extract_constructor_type, type.type);
}

bool is_composable(TypeConstructor const &lhs, TypeConstructor const &rhs) {
  if (lhs.type.empty() || rhs.type.empty())
    return true;

  try {
    throw_if_invalid_composition(lhs.type.back(), rhs.type.front());
    return true;
  } catch (std::exception &) {
    return false;
  }
}

TypeConstructor compose_type_constructors(TypeConstructor const &lhs,
                                          TypeConstructor const &rhs) {
  return TypeConstructor{compose_constructors(lhs.type, rhs.type)};
}

std::string to_string(TypeConstructor const &constructor,
                      std::vector<std::string> const &symbols) {
  return constructor_to_string(constructor, symbols);
}

std::string to_string(TypeConstructor::Type const &type,
                      std::vector<std::string> const &symbols) {
  return type_to_string(type, symbols);
}

} // namespace Types
} // namespace Project

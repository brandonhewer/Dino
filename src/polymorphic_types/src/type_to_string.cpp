#include "polymorphic_types/type_to_string.hpp"

#include <functional>
#include <numeric>

namespace {

using namespace Project::Types;

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
                           std::vector<std::string> const &,
                           std::vector<std::string> const &);

std::string outer_to_string(TypeConstructor::Type const &,
                            std::vector<std::string> const &,
                            std::vector<std::string> const &);

std::string
functor_variable_to_string(TypeConstructor::AtomicType const &type,
                           std::vector<std::string> const &symbols,
                           std::vector<std::string> const &functor_symbols) {
  return type_to_string(type.type, symbols, functor_symbols) +
         variance_to_string(type.variance);
}

template <typename StartIt, typename EndIt>
std::string types_to_string(StartIt start_it, EndIt end_it, Variance variance,
                            std::vector<std::string> const &symbols,
                            std::vector<std::string> const &functor_symbols) {
  std::string type_string;
  for (auto type_it = start_it; type_it < end_it; ++type_it) {
    auto const &type = *type_it;
    auto const delimiter = variance == Variance::CONTRAVARIANCE ? " -> " : " ";
    type_string +=
        delimiter + type_to_string(type.type, symbols, functor_symbols);
    variance = type.variance;
  }
  return std::move(type_string);
}

std::string
constructor_to_string(TypeConstructor::ConstructorType const &constructor,
                      std::vector<std::string> const &symbols,
                      std::vector<std::string> const &functor_symbols) {
  if (constructor.empty())
    return "";

  auto const &first_type = constructor.front();
  return type_to_string(first_type.type, symbols, functor_symbols) +
         types_to_string(constructor.begin() + 1, constructor.end(),
                         first_type.variance, symbols, functor_symbols);
}

std::string
constructor_to_string(TypeConstructor const &constructor,
                      std::vector<std::string> const &symbols,
                      std::vector<std::string> const &functor_symbols) {
  if (constructor.type.size() == 1)
    return outer_to_string(constructor.type[0].type, symbols, functor_symbols);
  return "(" +
         constructor_to_string(constructor.type, symbols, functor_symbols) +
         ")";
}

std::string
outer_constructor_to_string(TypeConstructor const &constructor,
                            std::vector<std::string> const &symbols,
                            std::vector<std::string> const &functor_symbols) {
  if (constructor.type.size() == 1)
    return outer_to_string(constructor.type[0].type, symbols, functor_symbols);
  return constructor_to_string(constructor.type, symbols, functor_symbols);
}

std::string functor_to_string(FunctorTypeConstructor const &functor,
                              std::vector<std::string> const &symbols,
                              std::vector<std::string> const &functor_symbols) {
  if (functor.identifier >= functor_symbols.size())
    return constructor_to_string(functor.type, symbols, functor_symbols);

  auto const add_type = [&](auto const &acc, auto const &type) {
    return acc + " " +
           functor_variable_to_string(type, symbols, functor_symbols);
  };
  return std::accumulate(functor.type.begin(), functor.type.end(),
                         functor_symbols[functor.identifier], add_type);
}

struct TypeToString {

  std::string operator()(std::vector<std::string> const &symbols,
                         std::vector<std::string> const &,
                         std::size_t identifier) const {
    return symbols[identifier];
  }

  std::string operator()(std::vector<std::string> const &,
                         std::vector<std::string> const &,
                         FreeType const &) const {
    return "*";
  }

  std::string operator()(std::vector<std::string> const &,
                         std::vector<std::string> const &,
                         MonoType const &type) const {
    return mono_type_to_string(type);
  }

  std::string operator()(std::vector<std::string> const &symbols,
                         std::vector<std::string> const &functor_symbols,
                         TypeConstructor const &constructor) const {
    return constructor_to_string(constructor, symbols, functor_symbols);
  }

  std::string operator()(std::vector<std::string> const &symbols,
                         std::vector<std::string> const &functor_symbols,
                         FunctorTypeConstructor const &functor) const {
    return "(" + functor_to_string(functor, symbols, functor_symbols) + ")";
  }

} _inner_to_string;

struct OuterToString {

  std::string operator()(std::vector<std::string> const &symbols,
                         std::vector<std::string> const &,
                         std::size_t identifier) const {
    return symbols[identifier];
  }

  std::string operator()(std::vector<std::string> const &,
                         std::vector<std::string> const &,
                         FreeType const &) const {
    return "*";
  }

  std::string operator()(std::vector<std::string> const &,
                         std::vector<std::string> const &,
                         MonoType const &type) const {
    return mono_type_to_string(type);
  }

  std::string operator()(std::vector<std::string> const &symbols,
                         std::vector<std::string> const &functor_symbols,
                         TypeConstructor const &constructor) const {
    return constructor_to_string(constructor, symbols, functor_symbols);
  }

  std::string operator()(std::vector<std::string> const &symbols,
                         std::vector<std::string> const &functor_symbols,
                         FunctorTypeConstructor const &functor) const {
    return functor_to_string(functor, symbols, functor_symbols);
  }

} _outer_to_string;

std::string type_to_string(TypeConstructor::Type const &type,
                           std::vector<std::string> const &symbols,
                           std::vector<std::string> const &functor_symbols) {
  return std::visit(std::bind(_inner_to_string, std::cref(symbols),
                              std::cref(functor_symbols),
                              std::placeholders::_1),
                    type);
}

std::string outer_to_string(TypeConstructor::Type const &type,
                            std::vector<std::string> const &symbols,
                            std::vector<std::string> const &functor_symbols) {
  return std::visit(std::bind(_outer_to_string, std::cref(symbols),
                              std::cref(functor_symbols),
                              std::placeholders::_1),
                    type);
}

} // namespace

namespace Project {
namespace Types {

std::string to_string(TypeConstructor const &constructor,
                      std::vector<std::string> const &symbols,
                      std::vector<std::string> const &functor_symbols) {
  return outer_constructor_to_string(constructor, symbols, functor_symbols);
}

std::string to_string(TypeConstructor::Type const &type,
                      std::vector<std::string> const &symbols,
                      std::vector<std::string> const &functor_symbols) {
  return type_to_string(type, symbols, functor_symbols);
}

} // namespace Types
} // namespace Project

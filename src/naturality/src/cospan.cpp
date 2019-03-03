#include "naturality/cospan.hpp"
#include "naturality/natural_transformation.hpp"

#include <algorithm>
#include <functional>

namespace {
using namespace Project::Naturality;
using namespace Project::Types;

CospanMorphism::MappedType
create_default_type(TypeConstructor::AtomicType const &);

CospanMorphism create_default(TypeConstructor::ConstructorType const &);

struct CreateDefaultMappedType {

  CospanMorphism::MappedType operator()(Variance variance,
                                        std::size_t identifier) const {
    return CospanMorphism::MappedType{0, variance};
  }

  CospanMorphism::MappedType
  operator()(Variance variance, FunctorTypeConstructor const &functor) const {
    return CospanMorphism::MappedType{create_default(functor.type), variance};
  }

  CospanMorphism::MappedType
  operator()(Variance variance, TypeConstructor const &type_constructor) const {
    return CospanMorphism::MappedType{create_default(type_constructor.type),
                                      variance};
  }

  template <typename T>
  CospanMorphism::MappedType operator()(Variance variance, T const &) const {
    return CospanMorphism::MappedType{EmptyType(), variance};
  }

} _create_default_mapped_type;

CospanMorphism::MappedType
create_default_type(TypeConstructor::AtomicType const &type) {
  auto const create_cospan_type = std::bind(
      _create_default_mapped_type, type.variance, std::placeholders::_1);
  return std::visit(create_cospan_type, type.type);
}

CospanMorphism
create_default(TypeConstructor::ConstructorType const &constructor) {
  CospanMorphism morphism;
  morphism.map.reserve(constructor.size());
  std::transform(constructor.begin(), constructor.end(),
                 std::back_inserter(morphism.map), create_default_type);
  return std::move(morphism);
}

CospanMorphism create_default_from(TypeConstructor const &constructor) {
  return create_default(constructor.type);
}

void get_cospan_values(std::vector<std::size_t> &, CospanMorphism const &);

struct GetCospanValues {

  void operator()(std::vector<std::size_t> &values, std::size_t value) const {
    values.emplace_back(value);
  }

  void operator()(std::vector<std::size_t> &values,
                  std::pair<std::size_t, std::size_t> const &value) const {
    values.emplace_back(value.first);
    values.emplace_back(value.second);
  }

  void operator()(std::vector<std::size_t> &values,
                  CospanMorphism const &morphism) const {
    get_cospan_values(values, morphism);
  }

  void operator()(std::vector<std::size_t> &, EmptyType const &) const {}
} _get_cospan_values;

void get_cospan_values(std::vector<std::size_t> &values,
                       CospanMorphism const &morphism) {
  auto const get =
      std::bind(_get_cospan_values, std::ref(values), std::placeholders::_1);
  for (auto &&cospan_type : morphism.map)
    std::visit(get, cospan_type.type);
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

std::string cospan_type_to_string(CospanMorphism::Type const &);

std::string type_substring(Variance variance,
                           CospanMorphism::MappedType const &type,
                           bool is_last) {
  if (variance == Variance::CONTRAVARIANCE)
    return " -> " + cospan_type_to_string(type.type) +
           (is_last ? variance_to_string(type.variance) : "");
  else
    return " " + cospan_type_to_string(type.type) +
           variance_to_string(type.variance);
}

template <typename StartIt, typename EndIt>
std::string types_to_string(StartIt start_it, EndIt end_it, Variance variance) {
  std::string cospan_string;
  for (auto type_it = start_it; type_it < end_it; ++type_it) {
    auto const &type = *type_it;
    cospan_string += type_substring(variance, type, type_it == end_it - 1);
    variance = type.variance;
  }
  return std::move(cospan_string);
}

std::string cospan_morphism_to_string(CospanMorphism const &morphism) {
  if (morphism.map.empty())
    return "";

  auto const &first_type = morphism.map.front();
  return cospan_type_to_string(first_type.type) +
         types_to_string(morphism.map.begin() + 1, morphism.map.end(),
                         first_type.variance);
}

struct CospanTypeToString {

  std::string operator()(std::size_t value) const {
    return std::to_string(value);
  }

  std::string
  operator()(std::pair<std::size_t, std::size_t> const &value) const {
    return std::to_string(value.first) + ":" + std::to_string(value.second);
  }

  std::string operator()(EmptyType const &) const { return "*"; }

  std::string operator()(CospanMorphism const &morphism) const {
    return "(" + cospan_morphism_to_string(morphism) + ")";
  }

} _cospan_type_to_string;

std::string cospan_type_to_string(CospanMorphism::Type const &type) {
  return std::visit(_cospan_type_to_string, type);
}

} // namespace

namespace Project {
namespace Naturality {

CospanStructure create_default_cospan(Types::TypeConstructor const &domain,
                                      Types::TypeConstructor const &codomain) {
  return CospanStructure{
      {create_default_from(domain), create_default_from(codomain)}};
}

std::vector<std::size_t> extract_cospan_values(CospanStructure const &cospan) {
  std::vector<std::size_t> values;
  for (auto &&domain : cospan.domains)
    get_cospan_values(values, domain);
  std::sort(values.begin(), values.end());
  values.erase(std::unique(values.begin(), values.end()), values.end());
  return std::move(values);
}

std::string to_string(CospanMorphism::Type const &type) {
  return cospan_type_to_string(type);
}

std::string to_string(CospanMorphism const &morphism) {
  return cospan_morphism_to_string(morphism);
}

std::string to_string(CospanStructure const &structure) {
  return cospan_morphism_to_string(structure.domains.front()) + " => " +
         cospan_morphism_to_string(structure.domains.back());
}

} // namespace Naturality
} // namespace Project
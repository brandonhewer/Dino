#include "naturality/cospan_to_string.hpp"

namespace {

using namespace Project::Naturality;
using namespace Project::Types;

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

#include "naturality/cospan_zip.hpp"
#include "naturality/cospan_equality.hpp"

namespace {

using namespace Project::Naturality;

struct ZipCospanTypes {

  CospanMorphism::Type operator()(std::size_t left, std::size_t right) const {
    return CospanMorphism::PairType{left, right};
  }

  CospanMorphism::Type operator()(CospanMorphism const &left,
                                  CospanMorphism const &right) const {
    return zip_cospan_morphisms(get_nested(left), get_nested(right));
  }

  CospanMorphism::Type operator()(CospanMorphism const &left,
                                  std::size_t right) const {
    if (auto const identifier = get_identifier(left))
      return CospanMorphism::PairType{*identifier, right};
    throw std::runtime_error(
        "attempted to zip cospans with differing structures");
  }

  CospanMorphism::Type operator()(std::size_t left,
                                  CospanMorphism const &right) const {
    if (auto const identifier = get_identifier(right))
      return CospanMorphism::PairType{left, *identifier};
    throw std::runtime_error(
        "attempted to zip cospans with differing structures");
  }

  CospanMorphism::Type operator()(EmptyType, EmptyType) { return EmptyType{}; }

  CospanMorphism::Type operator()(EmptyType, std::size_t right) {
    return CospanMorphism::PairType{0, right};
  }

  CospanMorphism::Type operator()(std::size_t left, EmptyType) {
    return CospanMorphism::PairType{left, 0};
  }

  template <typename T>
  CospanMorphism::Type operator()(CospanMorphism const &, T const &) const {
    throw std::runtime_error(
        "attempted to zip cospan morphism with invalid structure");
  }

  template <typename T>
  CospanMorphism::Type operator()(T const &, CospanMorphism const &) const {
    throw std::runtime_error(
        "attempted to zip cospan morphism with invalid structure");
  }

  template <typename T>
  CospanMorphism::Type operator()(T const &,
                                  CospanMorphism::PairType const &) const {
    throw std::runtime_error("unable to zip cospan pair type");
  }

  template <typename T>
  CospanMorphism::Type operator()(CospanMorphism::PairType const &,
                                  T const &) const {
    throw std::runtime_error("unable to zip cospan pair type");
  }

  template <typename T>
  CospanMorphism::Type operator()(T const &,
                                  CospanMorphism::PairType const &) const {
    throw std::runtime_error("unable to zip cospan pair type");
  }

} _zip_cospan_types;

} // namespace

namespace Project {
namespace Naturality {

CospanMorphism::Type zip_cospan_types(CospanMorphism::Type const &left,
                                      CospanMorphism::Type const &right) {
  return std::visit(_zip_cospan_types, left, right);
}

CospanMorphism zip_cospan_morphisms(CospanMorphism const &left,
                                    CospanMorphism const &right) {
  auto const expected_size = left.map.size();

  if (right.map.size() != expected_size)
    throw std::runtime_error(
        "attempted to zip cospans with differing internal morphisms");

  CospanMorphism zipped;
  zipped.map.reserve(expected_size);
  for (auto i = 0u; i < expected_size; ++i) {
    auto const left_type = left.map[i];
    zipped.map.emplace_back(CospanMorphism::MappedType{zip_cospan_types(
                                left_type.type, right.map[i].type)},
                            left_type.variance);
  }
  return std::move(zipped);
}

} // namespace Naturality
} // namespace Project
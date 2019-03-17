#include "naturality/cospan_zip.hpp"
#include "naturality/cospan_equality.hpp"

#include "naturality/cospan_to_string.hpp"

namespace {

using namespace Project::Naturality;

struct ZipCospanTypes {

  CospanMorphism::Type operator()(std::size_t left, std::size_t right) const {
    return CospanMorphism::PairType{left, right};
  }

  CospanMorphism::Type operator()(CospanMorphism const &left,
                                  CospanMorphism const &right) const {
    return zip_cospan_morphisms(left, right);
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

  CospanMorphism::Type operator()(EmptyType, EmptyType) const {
    return EmptyType{};
  }

  CospanMorphism::Type operator()(EmptyType, std::size_t right) const {
    return CospanMorphism::PairType{0, right};
  }

  CospanMorphism::Type operator()(std::size_t left, EmptyType) const {
    return CospanMorphism::PairType{left, 0};
  }

  template <typename T>
  CospanMorphism::Type operator()(CospanMorphism const &,
                                  CospanMorphism::PairType const &) const {
    throw std::runtime_error("attempted to zip cospan morphism with pair");
  }

  template <typename T>
  CospanMorphism::Type operator()(CospanMorphism::PairType const &,
                                  CospanMorphism const &) const {
    throw std::runtime_error("attempted to zip pair with cospan morphism");
  }

  CospanMorphism::Type operator()(CospanMorphism::PairType const &,
                                  CospanMorphism::PairType const &) const {
    throw std::runtime_error("unable to zip cospan pair types");
  }

  template <typename T, typename U>
  CospanMorphism::Type operator()(T const &, U const &) const {
    throw std::runtime_error("unable to zip cospans type");
  }

} _zip_cospan_types;

CospanMorphism zip_morphisms(CospanMorphism const &left,
                             CospanMorphism const &right) {
  auto const expected_size = left.map.size();

  if (right.map.size() != expected_size)
    throw std::runtime_error(
        "attempted to zip cospans with differing internal morphisms");

  CospanMorphism zipped;
  zipped.map.reserve(expected_size);
  for (auto i = 0u; i < expected_size; ++i) {
    auto const left_type = left.map[i];
    zipped.map.emplace_back(CospanMorphism::MappedType{
        zip_cospan_types(left_type.type, right.map[i].type),
        left_type.variance});
  }
  return std::move(zipped);
}

} // namespace

namespace Project {
namespace Naturality {

CospanMorphism::Type zip_cospan_types(CospanMorphism::Type const &left,
                                      CospanMorphism::Type const &right) {
  return std::visit(_zip_cospan_types, left, right);
}

CospanMorphism zip_cospan_morphisms(CospanMorphism const &left,
                                    CospanMorphism const &right) {
  return zip_morphisms(get_nested(left), get_nested(right));
}

} // namespace Naturality
} // namespace Project
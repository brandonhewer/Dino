#include "naturality/cospan_equality.hpp"

namespace {

using namespace Project::Naturality;

template <typename T> T *extract_type(CospanMorphism &type) {
  if (type.map.size() != 1)
    return nullptr;
  return std::get_if<T>(&type.map[0].type);
}

template <typename T> T const *extract_type(CospanMorphism const &type) {
  if (type.map.size() != 1)
    return nullptr;
  return std::get_if<T>(&type.map[0].type);
}

CospanMorphism const &extract_nested(CospanMorphism const &morphism) {
  auto nested = &morphism;
  while (auto next = extract_type<CospanMorphism>(*nested))
    nested = next;
  return *nested;
}

CospanMorphism &extract_nested(CospanMorphism &morphism) {
  auto nested = &morphism;
  while (auto next = extract_type<CospanMorphism>(*nested))
    nested = next;
  return *nested;
}

CospanMorphism const *extract_nested(CospanMorphism::Type const &type) {
  auto constructor = std::get_if<CospanMorphism>(&type);
  if (nullptr == constructor)
    return nullptr;
  return &extract_nested(*constructor);
}

template <typename T> T const *extract_type(CospanMorphism::Type const &type) {
  if (auto morphism = extract_nested(type))
    return extract_type<T>(*morphism);
  return std::get_if<T>(&type);
}

template <>
CospanMorphism const *extract_type(CospanMorphism::Type const &type) {
  if (auto morphism = extract_nested(type))
    return morphism;
  return nullptr;
}

bool is_equal_types(CospanMorphism::Type const &, CospanMorphism::Type const &);

bool is_equal_types(CospanMorphism::MappedType const &left,
                    CospanMorphism::MappedType const &right) {
  return left.variance == right.variance &&
         is_equal_types(left.type, right.type);
}

bool is_equal_morphisms(std::vector<CospanMorphism::MappedType> const &left,
                        std::vector<CospanMorphism::MappedType> const &right) {
  if (left.size() != right.size())
    return false;

  for (auto i = 0u; i < left.size(); ++i) {
    if (!is_equal_types(left[i], right[i]))
      return false;
  }
  return true;
}

bool is_equal_types(CospanMorphism const &left, CospanMorphism const &right) {
  auto const &left_nested = extract_nested(left);
  auto const &right_nested = extract_nested(right);
  return is_equal_morphisms(left_nested.map, right_nested.map);
}

bool is_equal_types(std::size_t left, std::size_t right) {
  return left == right;
}

bool is_equal_types(CospanMorphism::PairType const &left,
                    CospanMorphism::PairType const &right) {
  return left.first == right.first && left.second == right.second;
}

bool is_equal_types(EmptyType, EmptyType) { return true; }

template <typename T>
bool is_equal_to(T const &left, CospanMorphism const &morphism) {
  if (auto const right = extract_type<T>(extract_nested(morphism)))
    return is_equal_types(left, *right);
  return false;
}

struct IsEqualTypes {

  bool operator()(std::size_t left, std::size_t right) const {
    return is_equal_types(left, right);
  }

  bool operator()(CospanMorphism::PairType const &left,
                  CospanMorphism::PairType const &right) const {
    return is_equal_types(left, right);
  }

  bool operator()(EmptyType, EmptyType) const { return true; }

  bool operator()(CospanMorphism const &left,
                  CospanMorphism const &right) const {
    return is_equal_types(left, right);
  }

  template <typename T>
  bool operator()(T const &left, CospanMorphism const &morphism) const {
    return is_equal_to(left, morphism);
  }

  template <typename T>
  bool operator()(CospanMorphism const &morphism, T const &right) const {
    return is_equal_to(right, morphism);
  }

  template <typename T, typename U>
  bool operator()(T const &, U const &) const {
    return false;
  }

} _is_equal_types;

bool is_equal_types(CospanMorphism::Type const &left,
                    CospanMorphism::Type const &right) {
  return std::visit(_is_equal_types, left, right);
}

} // namespace

namespace Project {
namespace Naturality {

CospanMorphism &get_nested(CospanMorphism &morphism) {
  return extract_nested(morphism);
}

CospanMorphism const &get_nested(CospanMorphism const &morphism) {
  return extract_nested(morphism);
}

std::size_t const *get_identifier(CospanMorphism const &morphism) {
  return extract_type<std::size_t>(extract_nested(morphism));
}

CospanMorphism::PairType const *get_pair(CospanMorphism const &morphism) {
  return extract_type<CospanMorphism::PairType>(extract_nested(morphism));
}

bool is_equal(CospanMorphism const &left, CospanMorphism const &right) {
  return is_equal_types(left, right);
}

bool is_equal(CospanMorphism::Type const &type,
              CospanMorphism const &morphism) {
  return is_equal_types(*extract_type<CospanMorphism>(type), morphism);
}

bool is_equal(CospanMorphism::Type const &type, std::size_t identifier) {
  return *extract_type<std::size_t>(type) == identifier;
}

bool is_equal(CospanMorphism::Type const &type, EmptyType) {
  return nullptr != extract_type<EmptyType>(type);
}

bool is_equal(CospanMorphism::Type const &type,
              CospanMorphism::PairType const &types) {
  return is_equal_types(*extract_type<CospanMorphism::PairType>(type), types);
}

} // namespace Naturality
} // namespace Project

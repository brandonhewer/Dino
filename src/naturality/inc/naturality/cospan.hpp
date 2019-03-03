#ifndef __COSPAN_HPP_
#define __COSPAN_HPP_

#include "polymorphic_types/type_constructor.hpp"

#include <cstddef>
#include <string>
#include <variant>
#include <vector>

namespace Project {
namespace Naturality {

struct EmptyType {};

struct CospanMorphism {
  using Type = std::variant<std::size_t, std::pair<std::size_t, std::size_t>,
                            EmptyType, CospanMorphism>;
  using MappedType = Types::TypeWithVariance<Type>;
  std::vector<MappedType> map;
};

struct CospanStructure {
  std::vector<CospanMorphism> domains;
};

CospanStructure create_default_cospan(Types::TypeConstructor const &,
                                      Types::TypeConstructor const &);

std::vector<std::size_t> extract_cospan_values(CospanStructure const &cospan);

std::string to_string(CospanMorphism::Type const &type);
std::string to_string(CospanMorphism const &morphism);
std::string to_string(CospanStructure const &structure);

} // namespace Naturality
} // namespace Project

#endif
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
  std::size_t start_identifier;
  std::size_t number_of_identifiers;
};

CospanStructure create_default_cospan(Types::TypeConstructor const &,
                                      Types::TypeConstructor const &);

} // namespace Naturality
} // namespace Project

#endif
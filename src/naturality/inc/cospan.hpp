#ifndef __COSPAN_HPP_
#define __COSPAN_HPP_

#include <cstddef>
#include <variant>
#include <vector>

namespace Project {
namespace Naturality {

struct EmptyType {};

struct CospanStructure {
  using StructureType = std::variant<std::size_t, EmptyType, CospanStructure>;
  using MappedType = Types::TypeWithVariance<StructureType>;
  std::vector<MappedType> mapped_domain;
  std::vector<MappedType> mapped_codomain;
};

CospanStructure create_default_cospan(Types::TypeConstructor const &,
                                      Types::TypeConstructor const &);

} // namespace Naturality
} // namespace Project

#endif
#ifndef __NATURAL_TRANSFORMATION_HPP_
#define __NATURAL_TRANSFORMATION_HPP_

#include "polymorphic_types/type_constructor.hpp"

namespace Project {
namespace Naturality {

struct NaturalTransformation {
  Types::TypeConstructor domain;
  Types::TypeConstructor codomain;
  std::vector<std::string> symbols;
};

std::string to_string(NaturalTransformation const &transformation);

} // namespace Naturality
} // namespace Project

#endif
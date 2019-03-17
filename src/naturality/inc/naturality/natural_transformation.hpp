#ifndef __NATURAL_TRANSFORMATION_HPP_
#define __NATURAL_TRANSFORMATION_HPP_

#include "polymorphic_types/type_constructor.hpp"

namespace Project {
namespace Naturality {

struct NaturalTransformation {
  std::vector<Types::TypeConstructor> domains;
  std::vector<std::string> symbols;
  std::vector<std::string> functor_symbols;
};

std::string to_string(NaturalTransformation const &transformation);

std::string debug_string(NaturalTransformation const &transformation);

} // namespace Naturality
} // namespace Project

#endif
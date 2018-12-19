#ifndef __TYPES_HPP_
#define __TYPES_HPP_

#include <cstddef>

namespace Project {
namespace Types {

struct FreeType {};

enum class Variance { COVARIANCE, CONTRAVARIANCE };

enum class MonoType { INT, CHAR, FLOAT };

template <typename T> struct TypeWithVariance {
  T type;
  Variance variance;
};

} // namespace Types
} // namespace Project

#endif

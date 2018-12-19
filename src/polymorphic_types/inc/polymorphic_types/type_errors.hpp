#ifndef __TYPE_ERRORS_HPP_
#define __TYPE_ERRORS_HPP_

#include "polymorphic_types/types.hpp"

#include <stdexcept>

namespace Project {
namespace Types {

class CompositionError : public std::runtime_error {
public:
  CompositionError(Variance, Variance);
};

class EmptyTypeConstructorError : public std::runtime_error {
public:
  EmptyTypeConstructorError();
};

} // namespace Types
} // namespace Project

#endif

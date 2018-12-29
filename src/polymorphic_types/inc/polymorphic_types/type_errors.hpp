#ifndef __TYPE_ERRORS_HPP_
#define __TYPE_ERRORS_HPP_

#include "polymorphic_types/type_constructor.hpp"

#include <stdexcept>

namespace Project {
namespace Types {

class CompositionError : public std::runtime_error {
public:
  CompositionError(Variance, Variance);
};

class TypeMismatchError : public std::exception {
public:
  TypeMismatchError(TypeConstructor::Type const &,
                    TypeConstructor::Type const &);

  TypeConstructor::Type const &left() const;
  TypeConstructor::Type const &right() const;

private:
  TypeConstructor::Type const &m_lhs;
  TypeConstructor::Type const &m_rhs;
};

class InvalidTypeAccessError : public std::runtime_error {
public:
  InvalidTypeAccessError();
};

class EmptyTypeConstructorError : public std::runtime_error {
public:
  EmptyTypeConstructorError();
};

} // namespace Types
} // namespace Project

#endif

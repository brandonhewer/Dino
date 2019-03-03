#include "polymorphic_types/type_equality.hpp"

namespace {

using namespace Project::Types;

template <typename T>
T const *extract_type(TypeConstructor::ConstructorType const &type) {
  if (type.size() != 1)
    return nullptr;
  return std::get_if<T>(&type[0].type);
}

TypeConstructor const &extract_nested(TypeConstructor const &constructor) {
  auto nested = &constructor;
  while (auto next = extract_type<TypeConstructor>(nested->type))
    nested = next;
  return *nested;
}

TypeConstructor const *extract_nested(TypeConstructor::Type const &type) {
  auto constructor = std::get_if<TypeConstructor>(&type);
  if (nullptr == constructor)
    return nullptr;
  return &extract_nested(*constructor);
}

template <typename T> T const *extract_type(TypeConstructor::Type const &type) {
  if (auto constructor = extract_nested(type))
    return extract_type<T>(*constructor);
  return std::get_if<T>(&type);
}

template <>
TypeConstructor const *extract_type(TypeConstructor::Type const &type) {
  if (auto constructor = extract_nested(type))
    return constructor;
  return nullptr;
}

bool is_equal_types(TypeConstructor::Type const &,
                    TypeConstructor::Type const &);

bool is_equal_types(TypeConstructor::AtomicType const &left,
                    TypeConstructor::AtomicType const &right) {
  return left.variance == right.variance &&
         is_equal_types(left.type, right.type);
}

template <typename LStartIt, typename LEndIt, typename RStartIt,
          typename REndIt>
bool is_equal_constructors(LStartIt left_start, LEndIt left_end,
                           RStartIt right_start, REndIt right_end);

template <typename LStartIt, typename RStartIt>
bool is_equal_same_size(LStartIt left_start, RStartIt right_start,
                        std::size_t size) {
  for (auto i = 0u; i < size; ++i) {
    if (!is_equal_types(*(left_start + i), *(right_start + i)))
      return false;
  }
  return true;
}

template <typename LStartIt, typename SStartIt>
bool is_equal_different_size(LStartIt larger_start, SStartIt smaller_start,
                             std::size_t larger_size,
                             std::size_t smaller_size) {
  auto const last_small_index = smaller_size - 1;
  auto const smaller_constructor =
      extract_type<TypeConstructor>((smaller_start + last_small_index)->type);

  if (nullptr == smaller_constructor)
    return false;

  for (auto i = 0u; i < last_small_index; ++i) {
    if (!is_equal_types(*(larger_start + i), *(smaller_start + i)))
      return false;
  }
  return is_equal_constructors(
      larger_start + last_small_index, larger_start + larger_size,
      smaller_constructor->type.begin(), smaller_constructor->type.end());
}

template <typename LStartIt, typename LEndIt, typename RStartIt,
          typename REndIt>
bool is_equal_constructors(LStartIt left_start, LEndIt left_end,
                           RStartIt right_start, REndIt right_end) {
  auto const left_size = left_end - left_start;
  auto const right_size = right_end - right_start;

  if (left_size > right_size)
    return is_equal_different_size(left_start, right_start, left_size,
                                   right_size);
  else if (right_size > left_size)
    return is_equal_different_size(right_start, left_start, right_size,
                                   left_size);
  return is_equal_same_size(left_start, right_start, left_size);
}

bool is_equal_constructors(TypeConstructor::ConstructorType const &left,
                           TypeConstructor::ConstructorType const &right) {
  return is_equal_constructors(left.begin(), left.end(), right.begin(),
                               right.end());
}

bool is_equal_check(TypeConstructor const &left, TypeConstructor const &right) {
  auto const &left_nested = extract_nested(left);
  auto const &right_nested = extract_nested(right);
  return is_equal_constructors(left_nested.type, right_nested.type);
}

bool is_equal_check(FunctorTypeConstructor const &left,
                    FunctorTypeConstructor const &right) {
  if (left.identifier != right.identifier)
    return false;
  return is_equal_constructors(left.type, right.type);
}

bool is_equal_check(std::size_t left, std::size_t right) {
  return left == right;
}

bool is_equal_check(FreeType, FreeType) { return true; }

bool is_equal_check(MonoType left, MonoType right) { return left == right; }

template <typename T>
bool is_equal_to(T const &left, TypeConstructor const &constructor) {
  if (auto const right = extract_type<T>(extract_nested(constructor).type))
    return is_equal_check(left, *right);
  return false;
}

struct IsEqualTypes {

  bool operator()(FreeType, FreeType) const { return true; }

  bool operator()(MonoType left, MonoType right) const { return left == right; }

  bool operator()(std::size_t left, std::size_t right) const {
    return left == right;
  }

  bool operator()(TypeConstructor const &left,
                  TypeConstructor const &right) const {
    return is_equal_check(left, right);
  }

  bool operator()(FunctorTypeConstructor const &left,
                  FunctorTypeConstructor const &right) const {
    return is_equal_check(left, right);
  }

  template <typename T>
  bool operator()(T const &left, TypeConstructor const &constructor) const {
    return is_equal_to(left, constructor);
  }

  template <typename T>
  bool operator()(TypeConstructor const &constructor, T const &right) const {
    return is_equal_to(right, constructor);
  }

  template <typename T, typename U>
  bool operator()(T const &, U const &) const {
    return false;
  }

} _is_equal_types;

bool is_equal_types(TypeConstructor::Type const &left,
                    TypeConstructor::Type const &right) {
  return std::visit(_is_equal_types, left, right);
}

} // namespace

namespace Project {
namespace Types {

TypeConstructor const &get_nested(TypeConstructor const &constructor) {
  return extract_nested(constructor);
}

std::size_t const *get_identifier(TypeConstructor const &constructor) {
  return extract_type<std::size_t>(extract_nested(constructor));
}

bool is_equal(TypeConstructor const &left, TypeConstructor const &right) {
  return is_equal_check(left, right);
}

bool is_equal(TypeConstructor::Type const &type, std::size_t identifier) {
  return *extract_type<std::size_t>(type) == identifier;
}

bool is_equal(TypeConstructor::Type const &left, MonoType right) {
  return *extract_type<MonoType>(left) == right;
}

bool is_equal(TypeConstructor::Type const &left, FreeType) {
  return extract_type<FreeType>(left) != nullptr;
}

bool is_equal(FunctorTypeConstructor const &left,
              FunctorTypeConstructor const &right) {
  return is_equal_check(left, right);
}

bool is_equal(TypeConstructor::Type const &type,
              TypeConstructor const &constructor) {
  if (auto const left = extract_type<TypeConstructor>(type))
    return is_equal_check(*left, constructor);
  return false;
}

bool is_equal(TypeConstructor::Type const &type,
              FunctorTypeConstructor const &functor) {
  return false;
}

} // namespace Types
} // namespace Project

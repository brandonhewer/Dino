#include "naturality/cospan_unification.hpp"
#include "naturality/cospan_equality.hpp"

namespace {

using namespace Project::Naturality;

bool compute_unification(CospanMorphism const &, CospanMorphism const &,
                         CospanUnification &);

template <typename T>
bool unify_identifier(
    std::vector<std::optional<CospanMorphism::Type>> &unification,
    std::size_t identifier, T const &unifier) {
  if (auto const &current = unification[identifier])
    return is_equal(*current, unifier);
  unification[identifier] = unifier;
  return true;
}

struct ComputeUnification {

  bool operator()(CospanUnification &unification, std::size_t left,
                  std::size_t right) const {
    return unify_identifier(unification.left, left, right);
  }

  template <typename T>
  bool operator()(CospanUnification &unification, std::size_t identifier,
                  T const &type) const {
    return unify_identifier(unification.left, identifier, type);
  }

  template <typename T>
  bool operator()(CospanUnification &unification, T const &type,
                  std::size_t identifier) const {
    return unify_identifier(unification.right, identifier, type);
  }

  bool operator()(CospanUnification &unification, CospanMorphism const &left,
                  CospanMorphism const &right) const {
    return compute_unification(left, right, unification);
  }

  bool operator()(CospanUnification &unification, EmptyType, EmptyType) const {
    return true;
  }

  template <typename T, typename U>
  bool operator()(CospanUnification &, T const &, U const &) const {
    return false;
  }

} _compute_unification;

bool compute_unification(CospanMorphism::Type const &left,
                         CospanMorphism::Type const &right,
                         CospanUnification &unification) {
  return std::visit(std::bind(_compute_unification, std::ref(unification),
                              std::placeholders::_1, std::placeholders::_2),
                    left, right);
}

bool compute_unification(CospanMorphism::MappedType const &left,
                         CospanMorphism::MappedType const &right,
                         CospanUnification &unification) {
  return left.variance == right.variance &&
         compute_unification(left.type, right.type, unification);
}

bool compute_morphism_unification(CospanMorphism const &morphism,
                                  CospanMorphism::Type const &right,
                                  CospanUnification &unification) {
  return std::visit(std::bind(_compute_unification, std::ref(unification),
                              std::cref(morphism), std::placeholders::_1),
                    right);
}

bool compute_morphism_unification(CospanMorphism::Type const &left,
                                  CospanMorphism const &morphism,
                                  CospanUnification &unification) {
  return std::visit(std::bind(_compute_unification, std::ref(unification),
                              std::placeholders::_1, std::cref(morphism)),
                    left);
}

CospanMorphism
create_cospan_tail(std::vector<CospanMorphism::MappedType> const &types,
                   std::size_t from) {
  return {std::vector<CospanMorphism::MappedType>(types.begin() + from,
                                                  types.end())};
}

bool compute_unification_greater_right(
    std::vector<CospanMorphism::MappedType> const &left,
    std::vector<CospanMorphism::MappedType> const &right,
    CospanUnification &unification) {
  for (auto i = 0u; i < left.size() - 1; ++i) {
    if (!compute_unification(left[i], right[i], unification))
      return false;
  }
  return compute_morphism_unification(
      left.back().type, create_cospan_tail(right, left.size() - 1),
      unification);
}

bool compute_unification_greater_left(
    std::vector<CospanMorphism::MappedType> const &left,
    std::vector<CospanMorphism::MappedType> const &right,
    CospanUnification &unification) {
  for (auto i = 0u; i < right.size() - 1; ++i) {
    if (!compute_unification(left[i], right[i], unification))
      return false;
  }
  return compute_morphism_unification(
      create_cospan_tail(left, right.size() - 1), right.back().type,
      unification);
}

bool compute_unification_equal(
    std::vector<CospanMorphism::MappedType> const &left,
    std::vector<CospanMorphism::MappedType> const &right,
    CospanUnification &unification) {
  for (auto i = 0u; i < left.size(); ++i) {
    if (!compute_unification(left[i], right[i], unification))
      return false;
  }
  return true;
}

bool compute_unification(std::vector<CospanMorphism::MappedType> const &left,
                         std::vector<CospanMorphism::MappedType> const &right,
                         CospanUnification &unification) {
  if (left.size() > right.size())
    return compute_unification_greater_left(left, right, unification);
  else if (right.size() > left.size())
    return compute_unification_greater_right(left, right, unification);
  return compute_unification_equal(left, right, unification);
}

bool compute_unification(CospanMorphism const &left,
                         CospanMorphism const &right,
                         CospanUnification &unification) {
  auto const &left_nested = get_nested(left);
  auto const &right_nested = get_nested(right);
  return compute_unification(left_nested.map, right_nested.map, unification);
}

} // namespace

namespace Project {
namespace Naturality {

std::optional<CospanUnification>
calculate_unification(CospanMorphism const &left, CospanMorphism const &right,
                      std::size_t left_symbols, std::size_t right_symbols) {
  CospanUnification unification{
      std::vector<std::optional<CospanMorphism::Type>>(left_symbols,
                                                       std::nullopt),
      std::vector<std::optional<CospanMorphism::Type>>(right_symbols,
                                                       std::nullopt)};
  return compute_unification(left, right, unification)
             ? std::move(unification)
             : std::optional<CospanUnification>(std::nullopt);
}

} // namespace Naturality
} // namespace Project

#include "naturality/cospan_shared_count.hpp"

#include <limits>
#include <numeric>

#include "naturality/cospan_to_string.hpp"

namespace {

using namespace Project::Naturality;

std::pair<std::size_t, std::size_t>
right_min_max_in_cospan(CospanMorphism::Type const &);

std::pair<std::size_t, std::size_t>
right_min_max_in_morphism(CospanMorphism const &morphism) {
  auto const &types = morphism.map;
  auto min_max = right_min_max_in_cospan(types.front().type);

  for (auto type = types.begin() + 1; type < types.end(); ++type) {
    auto const [min, max] = right_min_max_in_cospan(type->type);
    min_max.first = std::min(min_max.first, min);
    min_max.second = std::max(min_max.second, max);
  }
  return min_max;
}

struct RightMinMaxInCospan {

  std::pair<std::size_t, std::size_t> operator()(std::size_t minimum,
                                                 std::size_t maximum,
                                                 std::size_t identifier) const {
    return {std::min(minimum, identifier), std::max(maximum, identifier)};
  }

  std::pair<std::size_t, std::size_t>
  operator()(std::size_t minimum, std::size_t maximum,
             CospanMorphism::PairType const &pair) const {
    return {std::min(minimum, pair.second), std::max(maximum, pair.second)};
  }

  std::pair<std::size_t, std::size_t>
  operator()(std::size_t minimum, std::size_t maximum,
             CospanMorphism const &morphism) const {
    auto [min, max] = right_min_max_in_morphism(morphism);
    return {std::min(minimum, min), std::max(maximum, max)};
  }

  std::pair<std::size_t, std::size_t>
  operator()(std::size_t minimum, std::size_t maximum, EmptyType) const {
    return {minimum, maximum};
  }

} _right_min_max_in_cospan;

std::pair<std::size_t, std::size_t>
right_min_max_in_cospan(CospanMorphism::Type const &type) {
  return std::visit(std::bind(_right_min_max_in_cospan,
                              std::numeric_limits<std::size_t>::max(), 0,
                              std::placeholders::_1),
                    type);
}

std::pair<std::size_t, std::size_t>
left_min_max_in_cospan(CospanMorphism::Type const &);

std::pair<std::size_t, std::size_t>
left_min_max_in_morphism(CospanMorphism const &morphism) {
  auto const &types = morphism.map;
  auto min_max = left_min_max_in_cospan(types.front().type);

  for (auto type = types.begin() + 1; type < types.end(); ++type) {
    auto const [min, max] = left_min_max_in_cospan(type->type);
    min_max.first = std::min(min_max.first, min);
    min_max.second = std::max(min_max.second, max);
  }
  return min_max;
}

struct LeftMinMaxInCospan {

  std::pair<std::size_t, std::size_t> operator()(std::size_t minimum,
                                                 std::size_t maximum,
                                                 std::size_t identifier) const {
    return {std::min(minimum, identifier), std::max(maximum, identifier)};
  }

  std::pair<std::size_t, std::size_t>
  operator()(std::size_t minimum, std::size_t maximum,
             CospanMorphism::PairType const &pair) const {
    return {std::min(minimum, pair.first), std::max(maximum, pair.first)};
  }

  std::pair<std::size_t, std::size_t>
  operator()(std::size_t minimum, std::size_t maximum,
             CospanMorphism const &morphism) const {
    auto [min, max] = left_min_max_in_morphism(morphism);
    return {std::min(minimum, min), std::max(maximum, max)};
  }

  std::pair<std::size_t, std::size_t>
  operator()(std::size_t minimum, std::size_t maximum, EmptyType) const {
    return {minimum, maximum};
  }

} _left_min_max_in_cospan;

std::pair<std::size_t, std::size_t>
left_min_max_in_cospan(CospanMorphism::Type const &type) {
  return std::visit(std::bind(_left_min_max_in_cospan,
                              std::numeric_limits<std::size_t>::max(), 0,
                              std::placeholders::_1),
                    type);
}

} // namespace

namespace Project {
namespace Naturality {

std::vector<std::pair<std::size_t, std::size_t>>
shared_count(std::vector<CospanMorphism> const &morphisms) {
  if (morphisms.size() <= 1)
    return {};

  std::vector<std::pair<std::size_t, std::size_t>> count_pairs;
  count_pairs.reserve(morphisms.size() - 1);
  count_pairs.emplace_back(0, 0);

  for (auto i = 0u; i < morphisms.size() - 1; ++i) {
    auto const [first_min, first_max] = right_min_max_in_morphism(morphisms[i]);
    auto const [second_min, second_max] =
        left_min_max_in_morphism(morphisms[i + 1]);
    auto const shared =
        1 + (std::max(first_max, second_max) - std::min(first_min, second_min));
    count_pairs[i].second = shared;
    count_pairs.emplace_back(shared, 0);
  }
  return std::move(count_pairs);
}

} // namespace Naturality
} // namespace Project

#ifndef __COSPAN_UNIFICATION_HPP_
#define __COSPAN_UNIFICATION_HPP_

#include "naturality/cospan.hpp"

#include <cstddef>
#include <optional>
#include <vector>

namespace Project {
namespace Naturality {

struct CospanUnification {
  std::vector<std::optional<CospanMorphism::Type>> left;
  std::vector<std::optional<CospanMorphism::Type>> right;
};

std::optional<CospanUnification>
calculate_unification(CospanMorphism const &left, CospanMorphism const &right,
                      std::size_t left_symbols, std::size_t right_symbols);

} // namespace Naturality
} // namespace Project

#endif

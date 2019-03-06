#include "naturality/cospan_renaming.hpp"

namespace {

using namespace Project::Naturality;

void shift_cospan_identifiers(std::size_t, CospanMorphism::Type &);

void shift_morphism_identifiers(std::size_t shift, CospanMorphism &morphism) {
  for (auto &&type : morphism.map)
    shift_cospan_identifiers(shift, type.type);
}

struct ShiftCospanIdentifiers {

  void operator()(std::size_t shift, std::size_t &identifier) const {
    identifier += shift;
  }

  void operator()(std::size_t shift,
                  std::pair<std::size_t, std::size_t> &identifiers) const {
    identifiers.first += shift;
    identifiers.second += shift;
  }

  void operator()(std::size_t shift, CospanMorphism &morphism) const {}

  void operator()(std::size_t shift, EmptyType) const {}

} _shift_cospan_identifiers;

void shift_cospan_identifiers(std::size_t shift, CospanMorphism::Type &type) {
  std::visit(std::bind(_shift_cospan_identifiers, shift, std::placeholders::_1),
             type);
}

} // namespace

namespace Project {
namespace Naturality {

CospanStructure &shift_identifiers(CospanStructure &cospan, std::size_t shift) {
  for (auto &&domain : cospan.domains)
    shift_morphism_identifiers(shift, domain);
  cospan.start_identifier += shift;
  return cospan;
}

} // namespace Naturality
} // namespace Project

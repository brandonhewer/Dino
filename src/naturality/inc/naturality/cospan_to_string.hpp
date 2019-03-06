#ifndef __COSPAN_TO_STRING_HPP_
#define __COSPAN_TO_STRING_HPP_

#include "naturality/cospan.hpp"

namespace Project {
namespace Naturality {

std::string to_string(CospanMorphism::Type const &type);
std::string to_string(CospanMorphism const &morphism);
std::string to_string(CospanStructure const &structure);

} // namespace Naturality
} // namespace Project

#endif

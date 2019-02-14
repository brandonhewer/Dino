#ifndef __COSPAN_PARSER_HPP_
#define __COSPAN_PARSER_HPP_

#include "naturality/cospan.hpp"

namespace Project {
namespace Types {

Naturality::CospanStructure parse_cospan(std::string const &cospan_string);

Naturality::CospanStructure parse_cospan(std::string const &domain,
                                         std::string const &codomain);

} // namespace Types
} // namespace Project

#endif

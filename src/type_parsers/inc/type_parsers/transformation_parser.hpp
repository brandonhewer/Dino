#ifndef __TRANSFORMATION_PARSER_HPP_
#define __TRANSFORMATION_PARSER_HPP_

#include "naturality/natural_transformation.hpp"

namespace Project {
namespace Types {

Naturality::NaturalTransformation
parse_transformation(std::string const &type_string);

Naturality::NaturalTransformation
parse_transformation(std::string const &domain, std::string const &codomain);

} // namespace Types
} // namespace Project

#endif

#ifndef __COSPAN_ZIP_HPP_
#define __COSPAN_ZIP_HPP_

#include "naturality/cospan.hpp"

namespace Project {
namespace Naturality {

CospanMorphism::Type zip_cospan_types(CospanMorphism::Type const &,
                                      CospanMorphism::Type const &);

CospanMorphism zip_cospan_morphisms(CospanMorphism const &,
                                    CospanMorphism const &);

} // namespace Naturality
} // namespace Project

#endif
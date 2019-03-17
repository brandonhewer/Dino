#ifndef __GRAPH_BUILDER_HPP_
#define __GRAPH_BUILDER_HPP_

#include "naturality/cospan.hpp"
#include "polymorphic_types/type_constructor.hpp"

#include <napi.h>

namespace Project {
namespace Naturality {

Napi::Value generate_graph(std::vector<Types::TypeConstructor> const &,
                           CospanStructure const &, std::size_t, std::size_t,
                           Napi::Env &);
}
} // namespace Project

#endif
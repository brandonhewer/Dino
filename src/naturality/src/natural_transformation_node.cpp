#include "naturality/natural_transformation_node.hpp"

namespace Project {
namespace Naturality {

NodeNaturalTransformation::NodeNaturalTransformation(
    Napi::CallbackInfo const &info)
    : Napi::ObjectWrap<NodeNaturalTransformation>(info),
      m_domain(info[0].ToObject()), m_codomain(info[1].ToObject()) {}

Napi::Function NodeNaturalTransformation::initialize(Napi::Env env) {}

Napi::Value NodeNaturalTransformation::domain(Napi::CallbackInfo const &info) {}

Napi::Value
NodeNaturalTransformation::codomain(Napi::CallbackInfo const &info) {}

Napi::Value NodeNaturalTransformation::type(Napi::CallbackInfo const &info) {}

Napi::Value NodeNaturalTransformation::graph(Napi::CallbackInfo const &info) {}

} // namespace Naturality
} // namespace Project

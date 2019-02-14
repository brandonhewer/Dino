#ifndef __NATURAL_TRANSFORMATION_NODE_HPP_
#define __NATURAL_TRANSFORMATION_NODE_HPP_

#include "naturality/cospan.hpp"
#include "naturality/natural_transformation.hpp"

#include <napi.h>

namespace Project {
namespace Naturality {

class NodeNaturalTransformation
    : public Napi::ObjectWrap<NodeNaturalTransformation> {
public:
  NodeNaturalTransformation(Napi::CallbackInfo const &);

  static Napi::Function initialize(Napi::Env);
  static Napi::Object create(Napi::CallbackInfo const &);

private:
  Napi::Value graph(Napi::CallbackInfo const &);
  Napi::Value string(Napi::CallbackInfo const &);
  Napi::Value cospan_string(Napi::CallbackInfo const &);
  Napi::Value set_cospan(Napi::CallbackInfo const &);

  static Napi::FunctionReference g_constructor;

  NaturalTransformation m_transformation;
  CospanStructure m_type;
};

} // namespace Naturality
} // namespace Project

#endif
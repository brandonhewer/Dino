#ifndef __NATURAL_TRANSFORMATION_NODE_HPP_
#define __NATURAL_TRANSFORMATION_NODE_HPP_

#include "naturality/cospan.hpp"

#include <napi.h>

namespace Project {
namespace Naturality {

class NodeNaturalTransformation
    : public Napi::ObjectWrap<NodeNaturalTransformation> {
public:
  NodeNaturalTransformation(Napi::CallbackInfo const &);

  static Napi::Function initialize(Napi::Env);

private:
  Napi::Value domain(Napi::CallbackInfo const &);
  Napi::Value codomain(Napi::CallbackInfo const &);
  Napi::Value type(Napi::CallbackInfo const &);
  Napi::Value graph(Napi::CallbackInfo const &);

  Napi::Object m_domain;
  Napi::Object m_codomain;
  CospanStructure m_type;
};

} // namespace Naturality
} // namespace Project

#endif
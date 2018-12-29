#ifndef __ATOMIC_TYPE_REFERENCE_HPP_
#define __ATOMIC_TYPE_REFERENCE_HPP_

#include "polymorphic_types/type_constructor.hpp"

#include <napi.h>

namespace Project {
namespace Types {

class NodeAtomicTypeReference
    : public Napi::ObjectWrap<NodeAtomicTypeReference> {
public:
  NodeAtomicTypeReference(Napi::CallbackInfo const &);

  static Napi::Function initialize(Napi::Env);
  static Napi::Object create(Napi::Env, Variance, TypeConstructor::Type &);

private:
  static Napi::FunctionReference g_constructor;

  Variance m_context_variance;
  TypeConstructor::Type *m_type;
};

} // namespace Types
} // namespace Project

#endif

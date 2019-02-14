#ifndef __TYPE_CONSTRUCTOR_NODE_HPP_
#define __TYPE_CONSTRUCTOR_NODE_HPP_

#include "polymorphic_types/type_constructor.hpp"

#include <napi.h>

namespace Project {
namespace Types {

class NodeTypeConstructor : public Napi::ObjectWrap<NodeTypeConstructor> {
public:
  NodeTypeConstructor(Napi::CallbackInfo const &);

  static Napi::Function initialize(Napi::Env);
  static Napi::Object create_type(Napi::CallbackInfo const &);
  static Napi::Object create_type(Napi::Object const &);
  static Napi::Object function_type(Napi::Env);
  static Napi::Object covariant_type(Napi::Env);
  static Napi::Object contravariant_type(Napi::Env);
  static Napi::Object product_type(Napi::Env, std::size_t);

  TypeConstructor const &type_constructor() const;
  void set_type_constructor(TypeConstructor &&type_constructor);

private:
  Napi::Value compose(Napi::CallbackInfo const &);
  Napi::Value functorise(Napi::CallbackInfo const &);
  Napi::Value type_at(Napi::CallbackInfo const &);

  static Napi::FunctionReference g_constructor;

  TypeConstructor m_type_constructor;
};

TypeConstructor create_type_constructor(Napi::Value const &value);

} // namespace Types
} // namespace Project

#endif
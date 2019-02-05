#ifndef __TYPE_ADDON_HPP_
#define __TYPE_ADDON_HPP_

#include <napi.h>

namespace Project {
namespace Types {

Napi::Object init_all(Napi::Env env, Napi::Object exports);
}
} // namespace Project

#endif

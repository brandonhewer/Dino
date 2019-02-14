#ifndef __NATURALITY_ADDON_HPP_
#define __NATURALITY_ADDON_HPP_

#include <napi.h>

namespace Project {
namespace Naturality {

Napi::Object init_all(Napi::Env env, Napi::Object exports);
}
} // namespace Project

#endif
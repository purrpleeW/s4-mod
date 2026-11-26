#pragma once
#include "engine/engine_common.hpp"

namespace t9 {
	enum class HksError : std::uint32_t {
		HKS_NO_ERROR = 0x00000000,
		HKS_GC_YIELD = 0x00000001,
		HKS_THROWING_ERROR = 0xFFFFFE0C,
		LUA_ERRMEM = 0xFFFFFF38,
		LUA_ERRRUN = 0xFFFFFF9C,
		LUA_ERRERR = 0xFFFFFED4,
		LUA_ERRFILE = 0xFFFFFFFB,
		LUA_ERRSYNTAX = 0xFFFFFFFC
	};
}

#pragma once

#include "engine/engine_common.hpp"
#include "engine/t10/MRef.hpp"
#include "engine/t10/TValue.hpp"

#include <cstddef>

namespace t10 {
	struct lua_StateSeasonSix {
		char pad_0000[0x18];
		MRef gl_ref_;
		char pad_0020[0x18];
		TValue* top_;
		char pad_0040[0x10];
		TValue* base_;
	};

	ENGINE_ASSERT_SZ(lua_StateSeasonSix, 0x58);
	static_assert(offsetof(lua_StateSeasonSix, gl_ref_) == 0x18);
	static_assert(offsetof(lua_StateSeasonSix, top_) == 0x38);
	static_assert(offsetof(lua_StateSeasonSix, base_) == 0x50);
}

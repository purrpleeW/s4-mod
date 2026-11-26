#pragma once
#include "engine/engine_common.hpp"
#include "engine/jup/GCRef.hpp"
#include "engine/jup/MRef.hpp"
#include "engine/jup/TValue.hpp"

namespace jup {
	class lua_State {
	private:
		char pad_0000[0x0010];	// 0x0000
	public:
		MRef gl_ref_;			// 0x0010
	private:
		char pad_0018[0x0020];	// 0x0018
	public:
		TValue* top_;			// 0x0038
	private:
		char pad_0040[0x0010];	// 0x0040
	public:
		TValue* base_;			// 0x0050
	};
	ENGINE_ASSERT_SZ(lua_State, 0x0058);
}

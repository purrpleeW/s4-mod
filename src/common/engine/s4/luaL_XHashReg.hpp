#pragma once
#include "engine/engine_common.hpp"
#include "engine/s4/lua_State.hpp"

namespace s4 {
	class luaL_XHashReg {
	public:
		XHash key_;							// 0x0000
		int(__fastcall* func_)(lua_State*);	// 0x0008
	};
	ENGINE_ASSERT_SZ(luaL_XHashReg, 0x0010);
}

#pragma once
#include "engine/engine_common.hpp"
#include "engine/t10/lua_State.hpp"

namespace t10 {
	class luaL_Reg {
	public:
		const char* name_;					// 0x0000
		int(__fastcall* func_)(lua_State*);	// 0x0008
	};
	ENGINE_ASSERT_SZ(luaL_Reg, 0x0010);
}

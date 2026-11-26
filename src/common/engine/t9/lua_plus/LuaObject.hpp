#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/HksObject.hpp"
#include "engine/t9/lua_plus/LuaState.hpp"

namespace t9::LuaPlus {
	class LuaObject {
	public:
		LuaState* state_;	// 0x0000
		HksObject object_;	// 0x0008
		LuaObject* next_;	// 0x0018
		LuaObject* prev_;	// 0x0020
	};
	ENGINE_ASSERT_SZ(LuaObject, 0x0028);
}

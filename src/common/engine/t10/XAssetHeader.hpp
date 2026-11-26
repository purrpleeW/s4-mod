#pragma once
#include "engine/engine_common.hpp"
#include "engine/t10/LuaFile.hpp"

namespace t10 {
	union XAssetHeader {
		LuaFile* lua_file_;	// 0x0000
		void* data_;		// 0x0000
	};
	ENGINE_ASSERT_SZ(XAssetHeader, 0x0008);
}

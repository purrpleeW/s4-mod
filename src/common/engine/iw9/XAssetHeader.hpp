#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw9/LocalizeEntry.hpp"
#include "engine/iw9/LuaFile.hpp"
#include "engine/iw9/MovieDef.hpp"

namespace iw9 {
	union XAssetHeader {
		LocalizeEntry* localize_;	// 0x0000
		LuaFile* lua_file_;			// 0x0000
		MovieDef* movie_;			// 0x0000
		void* data_;				// 0x0000
	};
	ENGINE_ASSERT_SZ(XAssetHeader, 0x0008);
}

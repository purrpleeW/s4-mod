#pragma once
#include "engine/engine_common.hpp"
#include "engine/jup/Camo.hpp"
#include "engine/jup/LocalizeEntry.hpp"
#include "engine/jup/LuaFile.hpp"
#include "engine/jup/MovieDef.hpp"

namespace jup {
	union XAssetHeader {
		Camo* camo_;				// 0x0000
		LocalizeEntry* localize_;	// 0x0000
		LuaFile* lua_file_;			// 0x0000
		MovieDef* movie_;			// 0x0000
		void* data_;				// 0x0000
	};
	ENGINE_ASSERT_SZ(XAssetHeader, 0x0008);
}

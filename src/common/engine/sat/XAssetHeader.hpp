#pragma once
#include "engine/engine_common.hpp"
#include "engine/sat/FontDef.hpp"
#include "engine/sat/GscObj.hpp"
#include "engine/sat/LocalizeEntry.hpp"
#include "engine/sat/LuaFile.hpp"
#include "engine/sat/MapInfoDef.hpp"
#include "engine/sat/RawFile.hpp"
#include "engine/sat/StringTable.hpp"
#include "engine/sat/TTFDef.hpp"
#include "engine/sat/WeaponCompleteDef.hpp"

namespace sat {
	union XAssetHeader {
		FontDef* font_def_;			// 0x0000
		GscObj* gsc_obj_;			// 0x0000
		LocalizeEntry* localize_;	// 0x0000
		LuaFile* lua_file_;			// 0x0000
		MapInfoDef* map_info_def_;	// 0x0000
		RawFile* raw_file_;			// 0x0000
		StringTable* string_table_;	// 0x0000
		TTFDef* ttf_def_;			// 0x0000
		WeaponCompleteDef* weapon_;	// 0x0000
		void* data_;				// 0x0000
	};
	ENGINE_ASSERT_SZ(XAssetHeader, 0x0008);
}

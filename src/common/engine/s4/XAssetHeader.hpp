#pragma once
#include "engine/engine_common.hpp"
#include "engine/s4/Camo.hpp"
#include "engine/s4/GfxImage.hpp"
#include "engine/s4/LocalizeEntry.hpp"
#include "engine/s4/LuaFile.hpp"
#include "engine/s4/RawFile.hpp"
#include "engine/s4/ScriptFile.hpp"
#include "engine/s4/StringTable.hpp"
#include "engine/s4/TTFDef.hpp"

namespace s4 {
	union XAssetHeader {
		Camo* camo_;
		GfxImage* image_;
		LocalizeEntry* localize_;	// 0x0000
		LuaFile* lua_file_;			// 0x0000
		RawFile* raw_file_;			// 0x0000
		ScriptFile* script_file_;	// 0x0000
		StringTable* string_table_;	// 0x0000
		TTFDef* ttf_def_;			// 0x0000
		void* data_;				// 0x0000
	};
	ENGINE_ASSERT_SZ(XAssetHeader, 0x0008);
}

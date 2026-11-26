#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/DB_AssetEntry.hpp"
#include "engine/iw8/XAssetHeader.hpp"

namespace iw8 {
	class DBMarkStackEntry {
	public:
		XAssetHeader asset_header_;		// 0x0000
		DB_AssetEntry* asset_entry_;	// 0x0008
	};
	ENGINE_ASSERT_SZ(DBMarkStackEntry, 0x0010);
}

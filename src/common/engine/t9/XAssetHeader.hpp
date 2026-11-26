#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/LocalizeEntry.hpp"

namespace t9 {
	union XAssetHeader {
		LocalizeEntry* localize_;	// 0x0000
		void* data_;				// 0x0000
	};
	ENGINE_ASSERT_SZ(XAssetHeader, 0x0008);
}

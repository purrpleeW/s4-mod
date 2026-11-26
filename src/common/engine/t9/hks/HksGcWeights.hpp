#pragma once
#include "engine/engine_common.hpp"

namespace t9::hks {
	struct HksGcWeights {
		HksGcCost remove_string_;			// 0x0000
		HksGcCost finalize_userdata_no_mm_;	// 0x0004
		HksGcCost finalize_userdata_gc_mm_;	// 0x0008
		HksGcCost clean_coroutine_;			// 0x000C
		HksGcCost remove_weak_;				// 0x0010
		HksGcCost mark_object_;				// 0x0014
		HksGcCost traverse_string_;			// 0x0018
		HksGcCost traverse_userdata_;		// 0x001C
		HksGcCost traverse_coroutine_;		// 0x0020
		HksGcCost traverse_weak_table_;		// 0x0024
		HksGcCost free_chunk_;				// 0x0028
		HksGcCost sweep_traverse_;			// 0x002C
	};
	ENGINE_ASSERT_SZ(HksGcWeights, 0x0030);
}

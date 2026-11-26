#pragma once
#include "engine/engine_common.hpp"
#include "engine/t8/DvarValue.hpp"

namespace t8 {
	class DvarData {
	public:
		DvarValue currrent_;	// 0x0000
		DvarValue latched_;		// 0x0020
		DvarValue reset_;		// 0x0040
	};
	ENGINE_ASSERT_SZ(DvarData, 0x0060);
}

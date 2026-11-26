#pragma once
#include "engine/engine_common.hpp"

namespace s4 {
	struct ScenarioZone {
		XHash xhash_key_;		// 0x0000
		XHash xhash_value_;		// 0x0008
	};
	ENGINE_ASSERT_SZ(ScenarioZone, 0x0010);
}

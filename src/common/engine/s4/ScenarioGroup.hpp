#pragma once
#include "engine/engine_common.hpp"

#include "engine/s4/Scenario.hpp"
#include "engine/s4/ScenarioZone.hpp"
#include "engine/s4/enums/ScenarioInstallState.hpp"

namespace s4 {
	struct ScenarioGroup {
		XHash xhash_;					// 0x0000
		const char* name_;				// 0x0008
		const char* description_;		// 0x0010
		std::uint32_t scenario_count_;	// 0x0018
		std::uint32_t zone_count_;		// 0x001C
		XHash(*scenarios_)[];			// 0x0020
		ScenarioZone(*zones_)[];		// 0x0028
	};
	ENGINE_ASSERT_SZ(ScenarioGroup, 0x0030);
}

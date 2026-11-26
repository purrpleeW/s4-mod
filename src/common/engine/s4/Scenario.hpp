#pragma once
#include "engine/engine_common.hpp"
#include "engine/s4/enums/ScenarioInstallState.hpp"

namespace s4 {
	struct Scenario {
		XHash xhash_;							// 0x0000
		const char* name_;						// 0x0008
	private:
		char gap0[0xC];							// 0x0010
	public:
		std::uint32_t platform_app_id_;			// 0x001C
	private:
		char gap20[0x4];						// 0x0020
	public:
		ScenarioInstallState install_state_;	// 0x0024
	};
	ENGINE_ASSERT_SZ(Scenario, 0x0028);
}

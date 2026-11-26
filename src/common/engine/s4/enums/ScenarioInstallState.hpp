#pragma once
#include "engine/engine_common.hpp"

namespace s4 {
	enum class ScenarioInstallState {
		NoOwnership = 0x0,
		Available = 0x1,
		Installing = 0x2,
		Installed = 0x3,
	};
}

#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class DBMemoryType {
		DM_MEMORY_VIRTUAL = 0,
		DM_MEMORY_SCRIPT = 1,
		DM_MEMORY_TEMP = 2,
		DM_MEMORY_GPUTEMP = 3,
		DM_MEMORY_COUNT = 4
	};
}

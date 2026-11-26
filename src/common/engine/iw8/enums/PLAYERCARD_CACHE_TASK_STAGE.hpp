#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class PLAYERCARD_CACHE_TASK_STAGE {
		PLAYERCARD_CACHE_TASK_STAGE_WAITING = 0,
		PLAYERCARD_CACHE_TASK_STAGE_WORKING = 1,
		PLAYERCARD_CACHE_TASK_STAGE_ALL_DONE = 2
	};
}

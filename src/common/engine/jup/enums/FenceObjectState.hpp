#pragma once
#include "engine/engine_common.hpp"

namespace jup {
	enum class FenceObjectState {
		STATE_IDLE = 1 << 0,
		STATE_ACTIVE = 1 << 1,
		STATE_PASSING = 1 << 2,
		STATE_FAILED = 1 << 3
	};
}

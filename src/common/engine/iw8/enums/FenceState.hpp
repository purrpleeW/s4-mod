#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class FenceState : std::int32_t {
		FENCE_INACTIVE = 0x0,
		FENCE_REQUESTED = 0x1,
		FENCE_DOWNLOADING = 0x2,
		FENCE_SUCCESS = 0x3,
		FENCE_FAILURE = 0x4,
		FENCE_QUEUED = 0x5,
		FENCE_REGISTERING = 0x6,
		FENCE_QUEUED_RETRY = 0x7
	};
}

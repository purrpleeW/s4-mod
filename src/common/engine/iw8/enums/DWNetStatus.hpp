#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class DWNetStatus {
		DW_NET_NOT_STARTED = 0,
		DW_NET_STARTING = 1,
		DW_NET_STARTED = 2,
		DW_NET_RELAY_STARTING = 3,
		DW_NET_RELAY_STARTED = 4,
		DW_NET_ERROR = 5
	};
}

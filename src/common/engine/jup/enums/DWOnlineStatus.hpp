#pragma once
#include "engine/engine_common.hpp"

namespace jup {
	enum class DWOnlineStatus {
		DW_LIVE_DISCONNECTED = 0,
		DW_LIVE_CONNECTING = 1,
		DW_LIVE_CONNECTED = 2
	};
}

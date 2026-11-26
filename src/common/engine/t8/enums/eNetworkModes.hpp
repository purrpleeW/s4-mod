#pragma once
#include "engine/engine_common.hpp"

namespace t8 {
	enum class eNetworkModes {
		MODE_NETWORK_OFFLINE = 0,
		MODE_NETWORK_SYSTEMLINK = 1,
		MODE_NETWORK_ONLINE = 2,
		MODE_NETWORK_COUNT = 3,
		MODE_NETWORK_INVALID = 3
	};
}

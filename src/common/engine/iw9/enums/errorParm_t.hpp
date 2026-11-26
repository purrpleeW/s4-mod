#pragma once
#include "engine/engine_common.hpp"

namespace iw9 {
	enum class errorParm_t {
		ERR_FATAL = 0,
		ERR_DROP = 1,
		ERR_SERVERDISCONNECT = 2,
		ERR_DISCONNECT = 3,
		ERR_SCRIPT = 4,
		ERR_SCRIPT_DROP = 5,
		ERR_LOCALIZATION = 6,
		ERR_COUNT = 7
	};
}

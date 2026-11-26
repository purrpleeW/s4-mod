#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class svscmd_type {
		SV_CMD_NONE = 0,
		SV_CMD_CAN_IGNORE = 1,
		SV_CMD_RELIABLE = 2
	};
}

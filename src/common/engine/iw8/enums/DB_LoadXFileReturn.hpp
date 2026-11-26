#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class DB_LoadXFileReturn {
		SUCCESS,
		PAUSED,
		CANCELLED,
		MISSING
	};
}

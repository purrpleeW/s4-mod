#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class MatchmakerUIState {
		IDLE = 0,
		INITIALIZING = 1,
		SEARCHING = 2,
		QOSING = 3,
		JOINED_LOBBY = 4,
		ERRORING = 5
	};
}

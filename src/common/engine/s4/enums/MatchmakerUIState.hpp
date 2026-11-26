#pragma once

namespace s4 {
	enum class MatchmakerUIState : int {
		IDLE = 0,
		INITIALIZING = 1,
		SEARCHING = 2,
		QOSING = 3,
		JOINED_LOBBY = 4,
		ERRORING = 5
	};
}
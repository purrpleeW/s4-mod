#pragma once
#include "engine/engine_common.hpp"

namespace t9 {
	enum class LobbyType {
		LOBBY_TYPE_INVALID = -1,
		LOBBY_TYPE_FIRST = 0,
		LOBBY_TYPE_PRIVATE = 0,
		LOBBY_TYPE_GAME = 1,
		LOBBY_TYPE_TRANSITION = 2,
		LOBBY_TYPE_LAST = 2,
		LOBBY_TYPE_AUTO = 3,
		LOBBY_TYPE_COUNT = 3
	};
}

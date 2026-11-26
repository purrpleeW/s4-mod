#pragma once
#include "engine/engine_common.hpp"

namespace t9 {
	enum class LobbyModule {
		LOBBY_MODULE_INVALID = -1,
		LOBBY_MODULE_FIRST = 0,
		LOBBY_MODULE_HOST = 0,
		LOBBY_MODULE_CLIENT = 1,
		LOBBY_MODULE_LAST = 1,
		LOBBY_MODULE_COUNT = 2,
		LOBBY_MODULE_PEER_TO_PEER = 3
	};
}

#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class OnlineChatType {
		SYSTEM_EVENT = 0,
		SYSTEM_ERROR = 1,
		PLAYER_EVENT = 2,
		PARTY_CHAT = 3,
		GAME_CHAT = 4,
		TEAM_CHAT = 5,
		CLAN_CHAT = 6,

		PRIVATE_CHAT_LEGACY = 7,

		CLAN_CHAT2 = 7,
		PRIVATE_CHAT = 8
	};
}

#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/DiscordActivity.hpp"
#include "engine/discord/DiscordUser.hpp"
#include "engine/discord/enums/EDiscordActivityActionType.hpp"

namespace discord {
	class IDiscordActivityEvents {
	public:
		void (*on_activity_join)(void* event_data, const char* secret);										// 0x0000
		void (*on_activity_spectate)(void* event_data, const char* secret);									// 0x0008
		void (*on_activity_join_request)(void* event_data, DiscordUser* user);								// 0x0010
		void (*on_activity_invite)(void* event_data, EDiscordActivityActionType type, DiscordUser* user,	// 0x0018
			DiscordActivity* activity);
	};
	ENGINE_ASSERT_SZ(IDiscordActivityEvents, 0x0020);
}

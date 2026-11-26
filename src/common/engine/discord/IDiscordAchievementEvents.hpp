#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/DiscordUserAchievement.hpp"

namespace discord {
	class IDiscordAchievementEvents {
	public:
		void (*on_user_achievement_update)(void* event_data, DiscordUserAchievement* user_achievement);	// 0x0000
	};
	ENGINE_ASSERT_SZ(IDiscordAchievementEvents, 0x0008);
}

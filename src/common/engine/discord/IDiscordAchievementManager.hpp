#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/DiscordUserAchievement.hpp"
#include "engine/discord/enums/EDiscordResult.hpp"

namespace discord {
	class IDiscordAchievementManager {
	public:
		void (*set_user_achievement)(IDiscordAchievementManager* manager, DiscordSnowflake achievement_id,						// 0x0000
			std::uint8_t percent_complete, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
		void (*fetch_user_achievements)(IDiscordAchievementManager* manager, void* callback_data,								// 0x0008
			void (*callback)(void* callback_data, EDiscordResult result));
		void (*count_user_achievements)(IDiscordAchievementManager* manager, int* count);										// 0x0010
		EDiscordResult (*get_user_achievement)(IDiscordAchievementManager* manager, DiscordSnowflake user_achievement_id,		// 0x0018
			DiscordUserAchievement* user_achievement);
		EDiscordResult (*get_user_achievement_at)(IDiscordAchievementManager* manager, int index,								// 0x0020
			DiscordUserAchievement* user_achievement);
	};
	ENGINE_ASSERT_SZ(IDiscordAchievementManager, 0x0028);
}

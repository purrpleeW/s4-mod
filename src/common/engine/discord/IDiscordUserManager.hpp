#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/DiscordUser.hpp"
#include "engine/discord/enums/EDiscordPremiumType.hpp"
#include "engine/discord/enums/EDiscordResult.hpp"
#include "engine/discord/enums/EDiscordUserFlag.hpp"

namespace discord {
	class IDiscordUserManager {
	public:
		EDiscordResult (*get_current_user)(IDiscordUserManager* manager, DiscordUser* current_user);						// 0x0000
		void (*get_user)(IDiscordUserManager* manager, DiscordUserId user_id, void* callback_data,							// 0x0008
			void (*callback)(void* callback_data, EDiscordResult result, DiscordUser* user));
		EDiscordResult (*get_current_user_premium_type)(IDiscordUserManager* manager, EDiscordPremiumType* premium_type);	// 0x0010
		EDiscordResult (*current_user_has_flag)(IDiscordUserManager* manager, EDiscordUserFlag flag, bool* has_flag);		// 0x0018
	};
	ENGINE_ASSERT_SZ(IDiscordUserManager, 0x0020);
}

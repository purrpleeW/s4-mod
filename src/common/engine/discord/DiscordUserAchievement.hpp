#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	class DiscordUserAchievement {
	public:
		DiscordSnowflake user_id_;			// 0x0000
		DiscordSnowflake achievement_id_;	// 0x0008
		std::uint8_t percent_complete_;		// 0x0010
	private:
		char pad_0011[0x0007];				// 0x0011
	public:
		DiscordDateTime unlocked_at_;		// 0x0018
	};
	ENGINE_ASSERT_SZ(DiscordUserAchievement, 0x0058);
}

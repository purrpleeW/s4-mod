#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/DiscordActivity.hpp"
#include "engine/discord/enums/EDiscordStatus.hpp"

namespace discord {
	class DiscordPresence {
	public:
		EDiscordStatus status;		// 0x0000
		DiscordActivity activity;	// 0x0004
	};
	ENGINE_ASSERT_SZ(DiscordPresence, 0x05B8);
}

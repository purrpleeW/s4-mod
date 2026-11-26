#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	class DiscordActivityTimestamps {
	public:
		DiscordTimestamp start_;	// 0x0000
		DiscordTimestamp end_;		// 0x0008
	};
	ENGINE_ASSERT_SZ(DiscordActivityTimestamps, 0x0010);
}

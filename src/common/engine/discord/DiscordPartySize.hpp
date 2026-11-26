#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	class DiscordPartySize {
	public:
		int current_size_;	// 0x0000
		int max_size_;		// 0x0004
	};
	ENGINE_ASSERT_SZ(DiscordPartySize, 0x0008);
}

#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	class DiscordRect {
	public:
		int left_;		// 0x0000
		int top_;		// 0x0004
		int right_;		// 0x0008
		int bottom_;	// 0x000C
	};
	ENGINE_ASSERT_SZ(DiscordRect, 0x0010);
}

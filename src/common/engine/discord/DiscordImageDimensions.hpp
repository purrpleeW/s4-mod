#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	class DiscordImageDimensions {
	public:
		std::uint32_t width_;	// 0x0000
		std::uint32_t height_;	// 0x0004
	};
	ENGINE_ASSERT_SZ(DiscordImageDimensions, 0x0008);
}

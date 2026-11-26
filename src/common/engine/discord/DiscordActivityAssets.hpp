#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	class DiscordActivityAssets {
	public:
		char large_image_[128];	// 0x0000
		char large_text_[128];	// 0x0080
		char small_image_[128];	// 0x0100
		char small_text_[128];	// 0x0180
	};
	ENGINE_ASSERT_SZ(DiscordActivityAssets, 0x0200);
}

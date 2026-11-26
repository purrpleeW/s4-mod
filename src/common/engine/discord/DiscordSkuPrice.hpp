#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	class DiscordSkuPrice {
	public:
		std::uint32_t amount_;	// 0x0000
		char currency_[16];		// 0x0004
	};
	ENGINE_ASSERT_SZ(DiscordSkuPrice, 0x0014);
}

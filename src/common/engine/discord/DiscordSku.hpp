#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/DiscordSkuPrice.hpp"
#include "engine/discord/enums/EDiscordSkuType.hpp"

namespace discord {
	class DiscordSku {
	public:
		DiscordSnowflake id_;	// 0x0000
		EDiscordSkuType type_;	// 0x0008
		char name_[256];		// 0x000C
		DiscordSkuPrice price_;	// 0x010C
	};
	ENGINE_ASSERT_SZ(DiscordSku, 0x0120);
}

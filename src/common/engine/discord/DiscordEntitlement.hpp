#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/enums/EDiscordEntitlementType.hpp"

namespace discord {
	class DiscordEntitlement {
	public:
		DiscordSnowflake id_;			// 0x0000
		EDiscordEntitlementType type_;	// 0x0008
	private:
		char pad_000C[0x0004];			// 0x000C
	public:
		DiscordSnowflake sku_id_;		// 0x0010
	};
	ENGINE_ASSERT_SZ(DiscordEntitlement, 0x0018);
}

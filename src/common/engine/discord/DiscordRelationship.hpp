#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/DiscordPresence.hpp"
#include "engine/discord/DiscordUser.hpp"
#include "engine/discord/enums/EDiscordRelationshipType.hpp"

namespace discord {
	class DiscordRelationship {
	public:
		EDiscordRelationshipType type_;	// 0x0000
	private:
		char pad_0004[0x0004];			// 0x0004
	public:
		DiscordUser user_;				// 0x0008
		DiscordPresence presence_;		// 0x01A0
	};
	ENGINE_ASSERT_SZ(DiscordRelationship, 0x0758);
}

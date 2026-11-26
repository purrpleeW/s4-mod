#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/DiscordPartySize.hpp"
#include "engine/discord/enums/EDiscordActivityPartyPrivacy.hpp"

namespace discord {
	class DiscordActivityParty {
	public:
		char id_[128];							// 0x0000
		DiscordPartySize size_;					// 0x0080
		EDiscordActivityPartyPrivacy privacy_;	// 0x0088
	};
	ENGINE_ASSERT_SZ(DiscordActivityParty, 0x008C);
}

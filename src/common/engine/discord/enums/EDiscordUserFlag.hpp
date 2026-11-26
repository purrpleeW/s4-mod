#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	enum class EDiscordUserFlag {
		DiscordUserFlag_Partner = 1 << 1,
		DiscordUserFlag_HypeSquadEvents = 1 << 2,
		DiscordUserFlag_HypeSquadHouse1 = 1 << 6,
		DiscordUserFlag_HypeSquadHouse2 = 1 << 7,
		DiscordUserFlag_HypeSquadHouse3 = 1 << 8
	};
}

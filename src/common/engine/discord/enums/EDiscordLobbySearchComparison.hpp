#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	enum class EDiscordLobbySearchComparison {
		DiscordLobbySearchComparison_LessThanOrEqual = -2,
		DiscordLobbySearchComparison_LessThan,
		DiscordLobbySearchComparison_Equal,
		DiscordLobbySearchComparison_GreaterThan,
		DiscordLobbySearchComparison_GreaterThanOrEqual,
		DiscordLobbySearchComparison_NotEqual
	};
}

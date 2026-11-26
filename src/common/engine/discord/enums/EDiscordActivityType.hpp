#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	enum class EDiscordActivityType {
		DiscordActivityType_Playing = 0,
		DiscordActivityType_Streaming = 1,
		DiscordActivityType_Listening = 2,
		DiscordActivityType_Watching = 3
	};
}

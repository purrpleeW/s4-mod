#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	enum class EDiscordStatus {
		DiscordStatus_Offline = 0,
		DiscordStatus_Online = 1,
		DiscordStatus_Idle = 2,
		DiscordStatus_DoNotDisturb = 3
	};
}

#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	enum class EDiscordActivitySupportedPlatformFlags {
		DiscordActivitySupportedPlatformFlags_Desktop = 1 << 0,
		DiscordActivitySupportedPlatformFlags_Android = 1 << 1,
		DiscordActivitySupportedPlatformFlags_iOS = 1 << 2
	};
}

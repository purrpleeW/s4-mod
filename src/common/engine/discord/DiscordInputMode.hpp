#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/enums/EDiscordInputModeType.hpp"

namespace discord {
	class DiscordInputMode {
		EDiscordInputModeType type_;	// 0x0000
		char shortcut_[256];			// 0x0004
	};
	ENGINE_ASSERT_SZ(DiscordInputMode, 0x0104);
}

#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	class DiscordActivitySecrets {
	public:
		char match_[128];		// 0x0000
		char join_[128];		// 0x0080
		char spectate_[128];	// 0x0100
	};
	ENGINE_ASSERT_SZ(DiscordActivitySecrets, 0x0180);
}

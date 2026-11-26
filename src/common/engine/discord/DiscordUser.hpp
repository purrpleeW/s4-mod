#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	class DiscordUser {
	public:
		DiscordUserId id_;		// 0x0000
		char username_[256];	// 0x0008
		char discriminator_[8];	// 0x0108
		char avatar_[128];		// 0x0110
		bool bot_;				// 0x0190
	private:
		char pad_0191[0x0007];	// 0x0191
	};
	ENGINE_ASSERT_SZ(DiscordUser, 0x0198);
}

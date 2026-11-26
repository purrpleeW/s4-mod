#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	class DiscordOAuth2Token {
	public:
		char access_token_[128];	// 0x0000
		char scopes_[1024];			// 0x0080
		DiscordTimestamp expires_;	// 0x0480
	};
	ENGINE_ASSERT_SZ(DiscordOAuth2Token, 0x0488);
}

#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/SocialPresence.hpp"
#include "engine/iw8/enums/CoDAuthState.hpp"

namespace iw8 {
	class LocalUserPresenceData {
	public:
		SocialPresence current_;			// 0x0000
		int last_update_;
		int next_update_;
		CoDAuthState auth_state_;
		char pad_0044[12];
	};
	ENGINE_ASSERT_SZ(LocalUserPresenceData, 0x0050);
}

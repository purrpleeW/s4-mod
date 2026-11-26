#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	class IDiscordUserEvents {
	public:
		void (*on_current_user_update)(void* event_data);
	};
	ENGINE_ASSERT_SZ(IDiscordUserEvents, 0x0008);
}

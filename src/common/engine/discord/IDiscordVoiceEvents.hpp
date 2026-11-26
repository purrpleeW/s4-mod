#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	class IDiscordVoiceEvents {
	public:
		void (*on_settings_update)(void* event_data);	// 0x0000
	};
	ENGINE_ASSERT_SZ(IDiscordVoiceEvents, 0x0008);
}

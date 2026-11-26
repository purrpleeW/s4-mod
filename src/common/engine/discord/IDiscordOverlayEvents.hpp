#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	class IDiscordOverlayEvents {
	public:
		void (*on_toggle)(void* event_data, bool locked);	// 0x0000
	};
	ENGINE_ASSERT_SZ(IDiscordOverlayEvents, 0x0008);
}

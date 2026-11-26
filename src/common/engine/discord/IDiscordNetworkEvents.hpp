#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	class IDiscordNetworkEvents {
	public:
		void (*on_message)(void* event_data, DiscordNetworkPeerId peer_id, DiscordNetworkChannelId channel_id,	// 0x0000
			std::uint8_t* data, std::uint32_t data_length);
		void (*on_route_update)(void* event_data, const char* route_data);										// 0x0008
	};
	ENGINE_ASSERT_SZ(IDiscordNetworkEvents, 0x0010);
}

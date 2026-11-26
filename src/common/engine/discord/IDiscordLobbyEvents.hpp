#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	class IDiscordLobbyEvents {
	public:
		void (*on_lobby_update)(void* event_data, std::int64_t lobby_id);													// 0x0000
		void (*on_lobby_delete)(void* event_data, std::int64_t lobby_id, std::uint32_t reason);								// 0x0008
		void (*on_member_connect)(void* event_data, std::int64_t lobby_id, std::int64_t user_id);							// 0x0010
		void (*on_member_update)(void* event_data, std::int64_t lobby_id, std::int64_t user_id);							// 0x0018
		void (*on_member_disconnect)(void* event_data, std::int64_t lobby_id, std::int64_t user_id);						// 0x0020
		void (*on_lobby_message)(void* event_data, std::int64_t lobby_id, std::int64_t user_id, std::uint8_t* data,			// 0x0028
			std::uint32_t data_length);
		void (*on_speaking)(void* event_data, std::int64_t lobby_id, std::int64_t user_id, bool speaking);					// 0x0030
		void (*on_network_message)(void* event_data, std::int64_t lobby_id, std::int64_t user_id, std::uint8_t channel_id,	// 0x0038
			std::uint8_t* data, std::uint32_t data_length);
	};
	ENGINE_ASSERT_SZ(IDiscordLobbyEvents, 0x0040);
}

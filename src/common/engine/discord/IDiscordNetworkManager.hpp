#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/enums/EDiscordResult.hpp"

namespace discord {
	class IDiscordNetworkManager {
	public:
		void (*get_peer_id)(IDiscordNetworkManager* manager, DiscordNetworkPeerId* peer_id);				// 0x0000
		EDiscordResult (*flush)(IDiscordNetworkManager* manager);										// 0x0008
		EDiscordResult (*open_peer)(IDiscordNetworkManager* manager, DiscordNetworkPeerId peer_id,		// 0x0010
			const char* route_data);
		EDiscordResult (*update_peer)(IDiscordNetworkManager* manager, DiscordNetworkPeerId peer_id,		// 0x0018
			const char* route_data);
		EDiscordResult (*close_peer)(IDiscordNetworkManager* manager, DiscordNetworkPeerId peer_id);		// 0x0020
		EDiscordResult (*open_channel)(IDiscordNetworkManager* manager, DiscordNetworkPeerId peer_id,	// 0x0028
			DiscordNetworkChannelId channel_id, bool reliable);
		EDiscordResult (*close_channel)(IDiscordNetworkManager* manager, DiscordNetworkPeerId peer_id,	// 0x0030
			DiscordNetworkChannelId channel_id);
		EDiscordResult (*send_message)(IDiscordNetworkManager* manager, DiscordNetworkPeerId peer_id,	// 0x0038
			DiscordNetworkChannelId channel_id, std::uint8_t* data, std::uint32_t data_length);
	};
	ENGINE_ASSERT_SZ(IDiscordNetworkManager, 0x0040);
}

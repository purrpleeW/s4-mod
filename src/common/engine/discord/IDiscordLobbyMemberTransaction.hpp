#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/enums/EDiscordResult.hpp"

namespace discord {
	class IDiscordLobbyMemberTransaction {
	public:
		EDiscordResult (*set_metadata)(IDiscordLobbyMemberTransaction* lobby_member_transaction, DiscordMetadataKey key,		// 0x0000
			DiscordMetadataValue value);
		EDiscordResult (*delete_metadata)(IDiscordLobbyMemberTransaction* lobby_member_transaction, DiscordMetadataKey key);	// 0x0008
	};
	ENGINE_ASSERT_SZ(IDiscordLobbyMemberTransaction, 0x0010);
}

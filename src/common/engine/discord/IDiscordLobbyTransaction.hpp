#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/enums/EDiscordLobbyType.hpp"
#include "engine/discord/enums/EDiscordResult.hpp"

namespace discord {
	class IDiscordLobbyTransaction {
	public:
		EDiscordResult (*set_type)(IDiscordLobbyTransaction* lobby_transaction, EDiscordLobbyType type);									// 0x0000
		EDiscordResult (*set_owner)(IDiscordLobbyTransaction* lobby_transaction, DiscordUserId owner_id);									// 0x0008
		EDiscordResult (*set_capacity)(IDiscordLobbyTransaction* lobby_transaction, uint32_t capacity);										// 0x0010
		EDiscordResult (*set_metadata)(IDiscordLobbyTransaction* lobby_transaction, DiscordMetadataKey key, DiscordMetadataValue value);	// 0x0018
		EDiscordResult (*delete_metadata)(IDiscordLobbyTransaction* lobby_transaction, DiscordMetadataKey key);								// 0x0020
		EDiscordResult (*set_locked)(IDiscordLobbyTransaction* lobby_transaction, bool locked);												// 0x0028
	};
	ENGINE_ASSERT_SZ(IDiscordLobbyTransaction, 0x0030);
}

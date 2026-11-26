#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/DiscordLobby.hpp"
#include "engine/discord/DiscordUser.hpp"
#include "engine/discord/IDiscordLobbyMemberTransaction.hpp"
#include "engine/discord/IDiscordLobbySearchQuery.hpp"
#include "engine/discord/IDiscordLobbyTransaction.hpp"
#include "engine/discord/enums/EDiscordResult.hpp"

namespace discord {
	class IDiscordLobbyManager {
	public:
		EDiscordResult (*get_lobby_create_transaction)(IDiscordLobbyManager* manager, IDiscordLobbyTransaction** transaction);									// 0x0000
		EDiscordResult (*get_lobby_update_transaction)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, IDiscordLobbyTransaction** transaction);			// 0x0008
		EDiscordResult (*get_member_update_transaction)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, DiscordUserId user_id,							// 0x0010
			IDiscordLobbyMemberTransaction** transaction);
		void (*create_lobby)(IDiscordLobbyManager* manager, IDiscordLobbyTransaction* transaction, void* callback_data,											// 0x0018
			void (*callback)(void* callback_data, EDiscordResult result, DiscordLobby* lobby));
		void (*update_lobby)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, IDiscordLobbyTransaction* transaction,										// 0x0020
			void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
		void (*delete_lobby)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, void* callback_data,														// 0x0028
			void (*callback)(void* callback_data, EDiscordResult result));
		void (*connect_lobby)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, DiscordLobbySecret secret, void* callback_data,							// 0x0030
			void (*callback)(void* callback_data, EDiscordResult result, DiscordLobby* lobby));
		void (*connect_lobby_with_activity_secret)(IDiscordLobbyManager* manager, DiscordLobbySecret activity_secret, void* callback_data,						// 0x0038
			void (*callback)(void* callback_data, EDiscordResult result, DiscordLobby* lobby));
		void (*disconnect_lobby)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, void* callback_data,													// 0x0040
			void (*callback)(void* callback_data, EDiscordResult result));
		EDiscordResult (*get_lobby)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, DiscordLobby* lobby);												// 0x0048
		EDiscordResult (*get_lobby_activity_secret)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, DiscordLobbySecret* secret);						// 0x0050
		EDiscordResult (*get_lobby_metadata_value)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, DiscordMetadataKey key,								// 0x0058
			DiscordMetadataValue* value);
		EDiscordResult (*get_lobby_metadata_key)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, int32_t index, DiscordMetadataKey* key);				// 0x0060
		EDiscordResult (*lobby_metadata_count)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, int32_t* count);											// 0x0068
		EDiscordResult (*member_count)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, int32_t* count);													// 0x0070
		EDiscordResult (*get_member_user_id)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, int32_t index, DiscordUserId* user_id);					// 0x0078
		EDiscordResult (*get_member_user)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, DiscordUserId user_id, DiscordUser* user);					// 0x0080
		EDiscordResult (*get_member_metadata_value)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, DiscordUserId user_id, DiscordMetadataKey key,		// 0x0088
			DiscordMetadataValue* value);
		EDiscordResult (*get_member_metadata_key)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, DiscordUserId user_id, int32_t index,					// 0x0090
			DiscordMetadataKey* key);
		EDiscordResult (*member_metadata_count)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, DiscordUserId user_id, int32_t* count);					// 0x0098
		void (*update_member)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, DiscordUserId user_id, IDiscordLobbyMemberTransaction* transaction,		// 0x00A0
			void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
		void (*send_lobby_message)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, std::uint8_t* data, std::uint32_t data_length, void* callback_data,	// 0x00A8
			void (*callback)(void* callback_data, EDiscordResult result));
		EDiscordResult (*get_search_query)(IDiscordLobbyManager* manager, IDiscordLobbySearchQuery** query);													// 0x00B0
		void (*search)(IDiscordLobbyManager* manager, IDiscordLobbySearchQuery* query, void* callback_data,														// 0x00B8
			void (*callback)(void* callback_data, EDiscordResult result));
		void (*lobby_count)(IDiscordLobbyManager* manager, int32_t* count);																						// 0x00C0
		EDiscordResult (*get_lobby_id)(IDiscordLobbyManager* manager, int32_t index, DiscordLobbyId* lobby_id);													// 0x00C8
		void (*connect_voice)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, void* callback_data,														// 0x00D0
			void (*callback)(void* callback_data, EDiscordResult result));
		void (*disconnect_voice)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, void* callback_data,													// 0x00D8
			void (*callback)(void* callback_data, EDiscordResult result));
		EDiscordResult (*connect_network)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id);																// 0x00E0
		EDiscordResult (*disconnect_network)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id);															// 0x00E8
		EDiscordResult (*flush_network)(IDiscordLobbyManager* manager);																							// 0x00F0
		EDiscordResult (*open_network_channel)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, std::uint8_t channel_id, bool reliable);					// 0x00F8
		EDiscordResult (*send_network_message)(IDiscordLobbyManager* manager, DiscordLobbyId lobby_id, DiscordUserId user_id, std::uint8_t channel_id,			// 0x0100
			std::uint8_t* data, std::uint32_t data_length);
	};
	ENGINE_ASSERT_SZ(IDiscordLobbyManager, 0x0108);
}

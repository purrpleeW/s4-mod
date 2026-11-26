#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/DiscordActivity.hpp"
#include "engine/discord/enums/EDiscordActivityActionType.hpp"
#include "engine/discord/enums/EDiscordActivityJoinRequestReply.hpp"
#include "engine/discord/enums/EDiscordResult.hpp"

namespace discord {
	class IDiscordActivityManager {
	public:
		EDiscordResult (*register_command)(IDiscordActivityManager* manager, const char* command);											// 0x0000
		EDiscordResult (*register_steam)(IDiscordActivityManager* manager, std::uint32_t steam_id);											// 0x0008
		void (*update_activity)(IDiscordActivityManager* manager, DiscordActivity* activity, void* callback_data,							// 0x0010
			void (*callback)(void* callback_data, EDiscordResult result));
		void (*clear_activity)(IDiscordActivityManager* manager, void* callback_data,														// 0x0018
			void (*callback)(void* callback_data, EDiscordResult result));
		void (*send_request_reply)(IDiscordActivityManager* manager, DiscordUserId user_id, EDiscordActivityJoinRequestReply reply,			// 0x0020
			void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
		void (*send_invite)(IDiscordActivityManager* manager, DiscordUserId user_id, EDiscordActivityActionType type, const char* content,	// 0x0028
			void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
		void (*accept_invite)(IDiscordActivityManager* manager, DiscordUserId user_id, void* callback_data,									// 0x0030
			void (*callback)(void* callback_data, EDiscordResult result));
	};
	ENGINE_ASSERT_SZ(IDiscordActivityManager, 0x0038);
}

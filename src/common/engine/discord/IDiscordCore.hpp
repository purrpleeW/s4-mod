#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/IDiscordAchievementManager.hpp"
#include "engine/discord/IDiscordActivityManager.hpp"
#include "engine/discord/IDiscordApplicationManager.hpp"
#include "engine/discord/IDiscordImageManager.hpp"
#include "engine/discord/IDiscordLobbyManager.hpp"
#include "engine/discord/IDiscordNetworkManager.hpp"
#include "engine/discord/IDiscordOverlayManager.hpp"
#include "engine/discord/IDiscordRelationshipManager.hpp"
#include "engine/discord/IDiscordStorageManager.hpp"
#include "engine/discord/IDiscordStoreManager.hpp"
#include "engine/discord/IDiscordUserManager.hpp"
#include "engine/discord/IDiscordVoiceManager.hpp"
#include "engine/discord/enums/EDiscordLogLevel.hpp"
#include "engine/discord/enums/EDiscordResult.hpp"

namespace discord {
	class IDiscordCore {
	public:
		void (*destroy)(IDiscordCore* core);													// 0x0000
		EDiscordResult (*run_callbacks)(IDiscordCore* core);									// 0x0008
		void (*set_log_hook)(IDiscordCore* core, EDiscordLogLevel min_level, void* hook_data,	// 0x0010
			void (*hook)(void* hook_data, EDiscordLogLevel level, const char* message));
		IDiscordApplicationManager* (*get_application_manager)(IDiscordCore* core);				// 0x0018
		IDiscordUserManager* (*get_user_manager)(IDiscordCore* core);							// 0x0020
		IDiscordImageManager* (*get_image_manager)(IDiscordCore* core);							// 0x0028
		IDiscordActivityManager* (*get_activity_manager)(IDiscordCore* core);					// 0x0030
		IDiscordRelationshipManager* (*get_relationship_manager)(IDiscordCore* core);			// 0x0038
		IDiscordLobbyManager* (*get_lobby_manager)(IDiscordCore* core);							// 0x0040
		IDiscordNetworkManager* (*get_network_manager)(IDiscordCore* core);						// 0x0048
		IDiscordOverlayManager* (*get_overlay_manager)(IDiscordCore* core);						// 0x0050
		IDiscordStorageManager* (*get_storage_manager)(IDiscordCore* core);						// 0x0058
		IDiscordStoreManager* (*get_store_manager)(IDiscordCore* core);							// 0x0060
		IDiscordVoiceManager* (*get_voice_manager)(IDiscordCore* core);							// 0x0068
		IDiscordAchievementManager* (*get_achievement_manager)(IDiscordCore* core);				// 0x0070
	};
	ENGINE_ASSERT_SZ(IDiscordCore, 0x0078);
}

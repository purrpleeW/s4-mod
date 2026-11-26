#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/IDiscordAchievementEvents.hpp"
#include "engine/discord/IDiscordActivityEvents.hpp"
#include "engine/discord/IDiscordLobbyEvents.hpp"
#include "engine/discord/IDiscordNetworkEvents.hpp"
#include "engine/discord/IDiscordOverlayEvents.hpp"
#include "engine/discord/IDiscordRelationshipEvents.hpp"
#include "engine/discord/IDiscordStoreEvents.hpp"
#include "engine/discord/IDiscordUserEvents.hpp"
#include "engine/discord/IDiscordVoiceEvents.hpp"

namespace discord {
	class DiscordCreateParams {
	public:
		DiscordClientId client_id_;							// 0x0000
		std::uint64_t flags_;								// 0x0008
		IDiscordCoreEvents* events_;						// 0x0010
		void* event_data_;									// 0x0018
		IDiscordApplicationEvents* application_events_;		// 0x0020
		DiscordVersion application_version_;				// 0x0028
	private:
		char pad_002C[0x0004];								// 0x002C
	public:
		IDiscordUserEvents* user_events_;					// 0x0030
		DiscordVersion user_version_;						// 0x0038
	private:
		char pad_003C[0x0004];								// 0x003C
	public:
		IDiscordImageEvents* image_events_;					// 0x0040
		DiscordVersion image_version_;						// 0x0048
	private:
		char pad_004C[0x0004];								// 0x004C
	public:
		IDiscordActivityEvents* activity_events_;			// 0x0050
		DiscordVersion activity_version_;					// 0x0058
	private:
		char pad_005C[0x0004];								// 0x005C
	public:
		IDiscordRelationshipEvents* relationship_events_;	// 0x0060
		DiscordVersion relationship_version_;				// 0x0068
	private:
		char pad_006C[0x0004];								// 0x006C
	public:
		IDiscordLobbyEvents* lobby_events_;					// 0x0070
		DiscordVersion lobby_version_;						// 0x0078
	private:
		char pad_007C[0x0004];								// 0x007C
	public:
		IDiscordNetworkEvents* network_events_;				// 0x0080
		DiscordVersion network_version_;					// 0x0088
	private:
		char pad_008C[0x0004];								// 0x008C
	public:
		IDiscordOverlayEvents* overlay_events_;				// 0x0090
		DiscordVersion overlay_version_;					// 0x0098
	private:
		char pad_009C[0x0004];								// 0x009C
	public:
		IDiscordStorageEvents* storage_events_;				// 0x00A0
		DiscordVersion storage_version_;					// 0x00A8
	private:
		char pad_00AC[0x0004];								// 0x00AC
	public:
		IDiscordStoreEvents* store_events_;					// 0x00B0
		DiscordVersion store_version_;						// 0x00B8
	private:
		char pad_00BC[0x0004];								// 0x00BC
	public:
		IDiscordVoiceEvents* voice_events_;					// 0x00C0
		DiscordVersion voice_version_;						// 0x00C8
	private:
		char pad_00CC[0x0004];								// 0x00CC
	public:
		IDiscordAchievementEvents* achievement_events_;		// 0x00D0
		DiscordVersion achievement_version_;				// 0x00D8
	private:
		char pad_00DC[0x0004];								// 0x00DC
	};
	ENGINE_ASSERT_SZ(DiscordCreateParams, 0x00E0);
}

#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/DiscordInputMode.hpp"
#include "engine/discord/enums/EDiscordResult.hpp"

namespace discord {
	class IDiscordVoiceManager {
	public:
		EDiscordResult (*get_input_mode)(IDiscordVoiceManager* manager, DiscordInputMode* input_mode);					// 0x0000
		void (*set_input_mode)(IDiscordVoiceManager* manager, DiscordInputMode input_mode, void* callback_data,			// 0x0008
			void (*callback)(void* callback_data, EDiscordResult result));
		EDiscordResult (*is_self_mute)(IDiscordVoiceManager* manager, bool* mute);										// 0x0010
		EDiscordResult (*set_self_mute)(IDiscordVoiceManager* manager, bool mute);										// 0x0018
		EDiscordResult (*is_self_deaf)(IDiscordVoiceManager* manager, bool* deaf);										// 0x0020
		EDiscordResult (*set_self_deaf)(IDiscordVoiceManager* manager, bool deaf);										// 0x0028
		EDiscordResult (*is_local_mute)(IDiscordVoiceManager* manager, DiscordSnowflake user_id, bool* mute);			// 0x0030
		EDiscordResult (*set_local_mute)(IDiscordVoiceManager* manager, DiscordSnowflake user_id, bool mute);			// 0x0038
		EDiscordResult (*get_local_volume)(IDiscordVoiceManager* manager, DiscordSnowflake user_id, uint8_t* volume);	// 0x0040
		EDiscordResult (*set_local_volume)(IDiscordVoiceManager* manager, DiscordSnowflake user_id, uint8_t volume);	// 0x0048
	};
	ENGINE_ASSERT_SZ(IDiscordVoiceManager, 0x0050);
}

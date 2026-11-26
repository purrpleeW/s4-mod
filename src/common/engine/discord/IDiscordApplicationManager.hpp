#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/DiscordOAuth2Token.hpp"
#include "engine/discord/enums/EDiscordResult.hpp"

namespace discord {
	class IDiscordApplicationManager {
	public:
		void (*validate_or_exit)(IDiscordApplicationManager* manager, void* callback_data,						// 0x0000
			void (*callback)(void* callback_data, EDiscordResult result));
		void (*get_current_locale)(IDiscordApplicationManager* manager, DiscordLocale* locale);					// 0x0008
		void (*get_current_branch)(IDiscordApplicationManager* manager, DiscordBranch* branch);					// 0x0010
		void (*get_oauth2_token)(IDiscordApplicationManager* manager, void* callback_data,						// 0x0018
			void (*callback)(void* callback_data, EDiscordResult result, DiscordOAuth2Token* oauth2_token));
		void (*get_ticket)(IDiscordApplicationManager* manager, void* callback_data,							// 0x0020
			void (*callback)(void* callback_data, EDiscordResult result, const char* data));
	};
	ENGINE_ASSERT_SZ(IDiscordApplicationManager, 0x0028);
}

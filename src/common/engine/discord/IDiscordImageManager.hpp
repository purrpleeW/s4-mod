#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/DiscordImageDimensions.hpp"
#include "engine/discord/DiscordImageHandle.hpp"
#include "engine/discord/enums/EDiscordResult.hpp"

namespace discord {
	class IDiscordImageManager {
	public:
		void (*fetch)(IDiscordImageManager* manager, DiscordImageHandle handle, bool refresh, void* callback_data,								// 0x0000
			void (*callback)(void* callback_data, EDiscordResult result, DiscordImageHandle handle_result));
		EDiscordResult (*get_dimensions)(IDiscordImageManager* manager, DiscordImageHandle handle, DiscordImageDimensions* dimensions);			// 0x0008
		EDiscordResult (*get_data)(IDiscordImageManager* manager, DiscordImageHandle handle, std::uint8_t* data, std::uint32_t data_length);	// 0x0010
	};
	ENGINE_ASSERT_SZ(IDiscordImageManager, 0x0018);
}

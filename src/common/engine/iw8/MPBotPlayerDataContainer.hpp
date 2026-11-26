#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/MPBotPlayerDataStrings.hpp"
#include "engine/iw8/MPBotPlayerData.hpp"

namespace iw8 {
	class MPBotPlayerDataContainer {
	public:
		MPBotPlayerDisplayStrings bot_display_data_;
		MPBotPlayerData bot_player_data_;
		bool is_data_valid_;
		bool sv_is_in_use_;
		char sv_client_index_;
	};
	ENGINE_ASSERT_SZ(MPBotPlayerDataContainer, 0x58);
}
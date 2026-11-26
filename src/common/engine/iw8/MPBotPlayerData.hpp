#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/ClientCustomizationInfo.hpp"

namespace iw8 {
	class MPBotPlayerData {
	public:
		std::uint16_t bot_name_index_;
		ClientCustomizationInfo bot_customization_;
		std::uint8_t bot_loadout_;
		std::uint8_t ai_team_;
		std::uint16_t ai_behavior_id_;
		std::int16_t ai_true_skill_rating_;
	};
	ENGINE_ASSERT_SZ(MPBotPlayerData, 0x24);
}
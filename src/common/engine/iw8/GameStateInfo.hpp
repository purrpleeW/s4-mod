#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/MatchRules.hpp"
#include "engine/iw8/LobbyMapRotationEntry.hpp"
#include "engine/iw8/MPBotPlayerDataContainer.hpp"

namespace iw8 {
	class GameStateInfo {
	public:
		bool using_recipe_; 							// 0x0000
	private:
		char pad_0001[7];								// 0x0001
	public:
		MatchRules* match_rules_; 						// 0x0008
		int using_rotation_;							// 0x0010
		int using_intermission_;						// 0x0014
		LobbyMapRotationEntry* map_rotation_;			// 0x0018
		std::uint8_t using_bots_connect_type_;			// 0x0020
		std::uint8_t using_bots_difficulty_[2];			// 0x0021
		std::uint8_t using_bots_team_limit_[2];			// 0x0023
	private:
		char pad_0024[4];								// 0x0024
	public:
		MPBotPlayerDataContainer bot_mm_info_[200];		// 0x0028
		bool mp_bots_enabled_game_launch_with_bots_; 	// 0x44E8
		bool mp_bot_data_initialized_;					// 0x44E9
		std::uint8_t agent_max_count_;					// 0x44EA
	};
	ENGINE_ASSERT_SZ(GameStateInfo, 0x44F0);
}
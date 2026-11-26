#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/bdSecurityID.hpp"
#include "engine/iw8/enums/VoteType.hpp"

namespace iw8 {
	class ClientAuthoritativeMemberInfo {
	public:
		bdSecurityID private_party_id_;				// 0x0000
		int available_map_pack_flags_;				// 0x0008
		std::uint32_t zombie_consumable0_;			// 0x000C
		std::uint32_t zombie_consumable1_;			// 0x0010
		std::uint32_t zombie_consumable2_;			// 0x0014
		std::uint32_t zombie_consumable3_;			// 0x0018
		std::uint32_t zombie_consumable4_;			// 0x001C
		std::uint32_t alien_ready_up_flag_;			// 0x0020
		std::uint32_t cp_field_upgrade_row_num_;	// 0x0024
		std::uint16_t alien_revives_;				// 0x0028
		std::uint16_t alien_escapes_;				// 0x002A
		std::uint16_t alien_nerf_escapes_;			// 0x002C
		std::uint16_t aliens_killed_;				// 0x002E
		int client_desired_team_selection_;			// 0x0030
		char gamertag_[36];							// 0x0034
		char clan_abbrev_[6];						// 0x0058
		std::uint8_t clan_tag_type_;				// 0x005E
		std::uint8_t player_card_update_;			// 0x005F
		bool headset_present_;						// 0x0060
		bool using_gamepad_;						// 0x0061
	private:
		char pad_0062[0x0002];						// 0x0062
	public:
		VoteType vote_;								// 0x0064
		int best_weapon_index_;						// 0x0068
		int wins_;									// 0x006C
		int kd_ratio_;								// 0x0070
		int current_win_streak_;					// 0x0074
		int games_played_;							// 0x0078
		int win_loss_ratio_;						// 0x007C
		int time_played_total_;						// 0x0080
		int true_skill_;							// 0x0084
		int mlg_spectator_;							// 0x0088
		int mlg_follower_;							// 0x008C
		int mlg_follower_index_;					// 0x0090
		int bnet_game_account_id_;					// 0x0094
		std::uint8_t platform_[1];					// 0x0098
	private:
		char pad_0099[0x0007];						// 0x0099
	public:
		std::uint64_t platform_id_;					// 0x00A0
		std::uint8_t kills_history_[5];				// 0x00A8
		std::uint8_t deaths_history_[5];			// 0x00AD
		bool is_headless_;							// 0x00B2
		bool game_battles_commit_ready_;			// 0x00B3
		bool server_overwrote_info_;				// 0x00B4
		bool is_paid_user_;							// 0x00B5
		bool has_online_subscription_;				// 0x00B6
	private:
		char pad_00B7[0x0001];						// 0x00B7
	};
	ENGINE_ASSERT_SZ(ClientAuthoritativeMemberInfo, 0x00B8);
}

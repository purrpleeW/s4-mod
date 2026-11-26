#include "common.hpp"
#include <identification/game.hpp>

namespace identification::game {
	game_iden get_target_info_game() {
		return game_iden{
			"s4-mod", "Vanguard", {
				{ { 0, 02, 7, 10140263 }, MP,  BattleNet, Beta, 0xAF79E042, XXH32 },	// 16/09/2021 23:04:55
				{ { 1, 07, 1, 10648059 }, AIO, BattleNet, Ship, 0x365E33D6, XXH32 },	// 18/11/2021 04:30:06
				{ { 1, 10, 0, 10819777 }, AIO, BattleNet, Ship, 0x357A41F0, XXH32 },	// 15/12/2021 04:48:47
				{ { 1, 14, 0, 11077351 }, AIO, BattleNet, Ship, 0xAB2F41BC, XXH32 },	// 17/02/2022 17:10:29
				{ { 1, 16, 0, 11283250 }, AIO, BattleNet, Ship, 0xAABADD37, XXH32 },	// 25/03/2022 01:28:08
				{ { 1, 24, 1, 12510470 }, CDL, BattleNet, Ship, 0x6399DB19, XXH32 },	// 11/10/2022 20:49:30
				{ { 1, 26, 1, 13860558 }, AIO, BattleNet, Ship, 0xCFFEE091, XXH32 },	// 29/01/2024 23:00:47
				{ { 1, 26, 1, 13860558 }, AIO, Steam,     Ship, 0xEB933916, XXH32 },	// 30/01/2024 00:54:55
				{ { 1, 27, 0, 26122688 }, AIO, BattleNet, Ship, 0xEAD6CB96, XXH32 },	// 05/03/2026 15:18:00
				{ { 1, 27, 0, 26122688 }, AIO, Steam, 	  Ship, 0x99CE422C, XXH32 },	// needs game boot
				{ { 1, 28, 0, 10000000 }, AIO, Steam, 	  Ship, 0x42E111F8, XXH32 }		// needs game boot
			}
		};
	}

	DEF_GAME_IMPL_SPECIFIC();
}

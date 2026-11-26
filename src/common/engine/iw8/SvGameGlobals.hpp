#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/scriptTimeStats_t.hpp"
#include "engine/iw8/ServerProfileTimes.hpp"
#include "engine/iw8/enums/scr_string_t.hpp"

namespace iw8 {
	class SvGameGlobals {
	public:
		int restarting_;						// 0x0000
		scr_string_t empty_config_string_;		// 0x0004
		scr_string_t* config_strings_;			// 0x0008
		int config_strings_count_;				// 0x0010
	private:
		char pad_0014[0x0004];					// 0x0014
	public:
		gentity_s* g_entities_;					// 0x0018
		int num_entities_;						// 0x0020
	private:
		char pad_0024[0x0004];					// 0x0024
	public:
		playerState_s* game_clients_;			// 0x0028
		int game_client_size_;					// 0x0030
		int skel_time_stamp_;					// 0x0034
		int skel_mem_pos_;						// 0x0038
	private:
		char pad_003C[0x0004];					// 0x003C
	public:
		char map_name_[64];						// 0x0040
		int map_crc_;							// 0x0080
		scriptTimeStats_t time_stats_;			// 0x0084
		ServerProfileTimes profile_;			// 0x00B8
	};
	ENGINE_ASSERT_SZ(SvGameGlobals, 0x0138);
}

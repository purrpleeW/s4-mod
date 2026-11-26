#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/pinglist_t.hpp"

namespace iw8 {
	class serverStatus_t {
	public:
		pinglist_t ping_list_[16];		// 0x00000000
		int num_queried_servers_;		// 0x00000440
		int current_ping_;				// 0x00000444
		int next_ping_time_;			// 0x00000448
		int max_servers_;				// 0x0000044C
		int refresh_time_;				// 0x00000450
		int num_servers_;				// 0x00000454
		int sort_key_;					// 0x00000458
		int sort_dir_;					// 0x0000045C
		int last_count_;				// 0x00000460
		int refresh_active_;			// 0x00000464
		int current_server_;			// 0x00000468
		int display_servers_[20000];	// 0x0000046C
		int num_display_servers_;		// 0x00013CEC
		int server_count_;				// 0x00013CF0
		int num_players_on_servers_;	// 0x00013CF4
		int next_display_refresh_;		// 0x00013CF8
		int next_sort_time_;			// 0x00013CFC
		int motd_len_;					// 0x00013D00
		int motd_width_;				// 0x00013D04
		int motd_paint_x_;				// 0x00013D08
		int motd_paint_x2_;				// 0x00013D0C
		int motd_offset_;				// 0x00013D10
		int motd_time_;					// 0x00013D14
		char cl_motd_[1024];			// 0x00013D18
	};
	ENGINE_ASSERT_SZ(serverStatus_t, 0x00014118);
}

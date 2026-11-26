#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class ServerProfileTimes {
	public:
		float frame_time_;					// 0x0000
		float wall_clock_time_;				// 0x0004
		float server_frame_time_min_;		// 0x0008
		float server_frame_time_max_;		// 0x000C
		float server_frame_time_target_;	// 0x0010
		float most_recent_frame_time_msec_;	// 0x0014
		int debug_frame_number_;			// 0x0018
		int server_time_;					// 0x001C
		float server_time_frames_;			// 0x0020
		float server_time_accum_;			// 0x0024
		float vm_frame_time_frames_;		// 0x0028
		float vm_frame_time_;				// 0x002C
		float vm_frame_time_min_;			// 0x0030
		float vm_frame_time_max_;			// 0x0034
		float vm_frame_time_accum_;			// 0x0038
		float vm_frame_time_min_accum_;		// 0x003C
		float vm_frame_time_max_accum_;		// 0x0040
		float vm_builtin_time_;				// 0x0044
		float vm_builtin_time_accum_;		// 0x0048
		float vm_builtin_time_min_;			// 0x004C
		float vm_builtin_time_max_;			// 0x0050
		float vm_builtin_time_min_accum_;	// 0x0054
		float vm_builtin_time_max_accum_;	// 0x0058
		float vm_ent_field_time_;			// 0x005C
		float vm_ent_field_time_accum_;		// 0x0060
		float wait_start_time_;				// 0x0064
		float wait_client_time_;			// 0x0068
		float wait_timeout_time_;			// 0x006C
		float wait_send_time_;				// 0x0070
		int anim_calls_;					// 0x0074
		int anim_calls_min_;				// 0x0078
		int anim_calls_max_;				// 0x007C
	};
	ENGINE_ASSERT_SZ(ServerProfileTimes, 0x0080);
}

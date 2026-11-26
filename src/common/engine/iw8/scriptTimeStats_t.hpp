#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class scriptTimeStats_t {
	public:
		float server_script_time_total_;								// 0x0000
		float server_script_time_exceed_total_;							// 0x0004
		float server_script_profile_calc_time_total_;					// 0x0008
		float server_script_profile_calc_time_max_;						// 0x000C
		std::uint32_t server_script_profile_calc_time_max_at_frame_;	// 0x0010
		float server_script_time_max_;									// 0x0014
		std::uint32_t server_script_usage_op_total_;					// 0x0018
		std::uint32_t server_script_usage_op_count_max_;				// 0x001C
		std::uint32_t server_script_usage_thread_resume_total_;			// 0x0020
		std::uint32_t server_script_usage_thread_resume_max_;			// 0x0024
		float server_script_usage_thread_create_time_total_;			// 0x0028
		std::uint32_t server_script_usage_thread_create_time_count_;	// 0x002C
		float server_script_usage_thread_create_time_max_;				// 0x0030
	};
	ENGINE_ASSERT_SZ(scriptTimeStats_t, 0x0034);
}

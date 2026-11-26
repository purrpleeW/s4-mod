#pragma once
#include "engine/engine_common.hpp"

namespace t9::hks {
	class RuntimeProfileData {
	public:
		class Stats {
		public:
			hksUint64 hks_time_;						// 0x0000
			hksUint64 callback_time_;					// 0x0008
			hksUint64 gc_time_;							// 0x0010
			hksUint64 c_finalizer_time_;				// 0x0018
			hksUint64 compiler_time_;					// 0x0020
			hksUint32 hkss_time_samples_;				// 0x0028
			hksUint32 callback_time_samples_;			// 0x002C
			hksUint32 gc_time_samples_;					// 0x0030
			hksUint32 compiler_time_samples_;			// 0x0034
			hksUint32 num_newuserdata_;					// 0x0038
			hksUint32 num_tablerehash_;					// 0x003C
			hksUint32 num_pushstring_;					// 0x0040
			hksUint32 num_pushcfunction_;				// 0x0044
			hksUint32 num_newtables_;					// 0x0048
		private:
			char pad_004C[0x0004];						// 0x004C
		};

		hksInt64 stack_depth_;							// 0x0000
		hksInt64 callback_depth_;						// 0x0008
		hksUint64 last_timer_;							// 0x0010
		Stats frame_stats_;								// 0x0018
		hksUint64 gc_start_time_;						// 0x0068
		hksUint64 finalizer_start_time_;				// 0x0070
		hksUint64 compiler_start_time_;					// 0x0078
		hksUint64 compiler_start_gc_time_;				// 0x0080
		hksUint64 compiler_start_gc_finalizer_time_;	// 0x0088
		hksUint64 compiler_callback_start_time_;		// 0x0090
		hksInt64 compiler_depth_;						// 0x0098
		void* out_file_;								// 0x00A0
		lua_State* root_state_;							// 0x00A8
	};
	ENGINE_ASSERT_SZ(RuntimeProfileData, 0x00B0);
}

#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/hksInstruction.hpp"

namespace t9::hks {
	class DebugInstance {
	public:
		class RuntimeProfilerStats {
		public:
			hksInt32 hks_time_;									// 0x0000
			hksInt32 callback_time_;							// 0x0004
			hksInt32 gc_time_;									// 0x0008
			hksInt32 c_finalizer_time_;							// 0x000C
			hksInt64 heap_size_;								// 0x0010
			hksInt64 num_newuserdata_;							// 0x0018
			hksInt64 num_pushstring_;							// 0x0020
		};

		int saved_objects_;										// 0x0000
		int keep_alive_objects_;								// 0x0004
		lua_State* active_state_;								// 0x0008
		lua_State* main_state_;									// 0x0010
		Debugger* owner_;										// 0x0018
		hksInt32 debugger_devel_;								// 0x0020
		hksInt32 stored_hook_level_;							// 0x0024
		bool clear_hook_;										// 0x0028
	private:
		char pad_0029[0x0007];									// 0x0029
	public:
		const hksInstruction* stored_hook_return_addr_;			// 0x0030
		hksInt32 debug_step_last_line_;							// 0x0038
	private:
		char pad_003C[0x0004];									// 0x003C
	public:
		DebugInstance* next_;									// 0x0040
		const hksInstruction* active_pc_;						// 0x0048
		hksInt32 runtime_profile_send_buffer_write_position_;	// 0x0050
	private:
		char pad_0054[0x0004];									// 0x0054
	public:
		RuntimeProfilerStats runtime_profile_send_buffer_[30];	// 0x0058
	};
	ENGINE_ASSERT_SZ(DebugInstance, 0x0508);
}

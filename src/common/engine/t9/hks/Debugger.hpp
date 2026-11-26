#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/hksInstruction.hpp"
#include "engine/t9/hks/BreakpointList.hpp"
#include "engine/t9/hks/DebugInstance.hpp"

namespace t9::hks {
	class Debugger {
	public:
		enum class RunMode {
			RUN_CONTINUE = 0,
			RUN_STEP_IN = 1,
			RUN_STEP_OUT = 2,
			RUN_STEP_OVER = 3
		};

		int (**v_ptr_)(void);							// 0x0000
		const hksInstruction* entry_pc_;				// 0x0008
		hks::Debugger::RunMode run_mode_;				// 0x0010
		hksInt32 target_callstack_level_;				// 0x0014
		lua_State* target_state_;						// 0x0018
		hksInt32 stack_level_line_num_tracking_[64];	// 0x0020
		lua_Alloc allocator_function_;					// 0x0120
		void* allocator_user_data_;						// 0x0128
		hksSize allocated_;								// 0x0130
		hks::BreakpointList breakpoints_;				// 0x0138
		hks::DebugInstance* head_;						// 0x0548
		lua_State* profiler_state_;						// 0x0550
		hksUint64 last_time_;							// 0x0558
		hksBool toggle_profile_;						// 0x0560
	private:
		char pad_0564[0x0004];							// 0x0564
	};
	ENGINE_ASSERT_SZ(Debugger, 0x0568);
}

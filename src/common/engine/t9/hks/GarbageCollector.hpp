#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/HksObject.hpp"
#include "engine/t9/hks/HksGcWeights.hpp"
#include "engine/t9/hks/MemoryManager.hpp"
#include "engine/t9/hks/ResumeData_Entry.hpp"
#include "engine/t9/hks/WeakStack_Entry.hpp"

namespace t9::hks {
	class GarbageCollector {
	public:
		class ResumeStack {
		public:
			ResumeData_Entry* storage_;			// 0x0000
			hksInt32 num_entries_;				// 0x0008
			hksUint32 num_allocated_;			// 0x000C
		};

		class GreyStack {
		public:
			HksObject* storage_;				// 0x0000
			hksSize num_entries_;				// 0x0008
			hksSize num_allocated_;				// 0x0010
		};

		class RemarkStack {
		public:
			HashTable** storage_;				// 0x0000
			hksSize num_allocated_;				// 0x0008
			hksSize num_entries_;				// 0x0010
		};

		class WeakStack {
		public:
			WeakStack_Entry* storage_;			// 0x0000
			hksInt32 num_entries_;				// 0x0008
			hksUint32 num_allocated_;			// 0x000C
		};

		HksGcCost target_;						// 0x0000
		HksGcCost steps_left_;					// 0x0004
		HksGcCost step_limit_;					// 0x0008
		HksGcWeights costs_;					// 0x000C
		HksGcCost unit_;						// 0x003C
		jmp_buf* jump_point_;					// 0x0040
		lua_State* main_state_;					// 0x0048
		lua_State* finalizer_state_;			// 0x0050
		MemoryManager* memory_;					// 0x0058
		void* emergency_gc_memory_;				// 0x0060
		hksInt32 phase_;						// 0x0068
	private:
		char pad_006C[0x0004];					// 0x006C
	public:
		ResumeStack resume_stack_;				// 0x0070
		GreyStack grey_stack_;					// 0x0080
		hksInt32 init_grey_stack_size_;			// 0x0098
	private:
		char pad_009C[0x0004];					// 0x009C
	public:
		RemarkStack remark_stack_;				// 0x00A0
		WeakStack weak_stack_;					// 0x00B8
		hksBool finalizing_;					// 0x00C8
	private:
		char pad_00CC[0x0004];					// 0x00CC
	public:
		HksObject safe_table_value;				// 0x00D0
		lua_State* start_of_state_stack_list_;	// 0x00E0
		lua_State* end_of_state_stack_list_;	// 0x00E8
		lua_State* current_state_;				// 0x00F0
		HksObject safe_value_;					// 0x00F8
		void* compiler_;						// 0x0108
		void* bytecode_reader_;					// 0x0110
		void* bytecode_writer_;					// 0x0118
		hksInt32 pause_multiplier_;				// 0x0120
		HksGcCost step_mulitplier_;				// 0x0124
		hksSize emergency_memory_size_;			// 0x0128
		bool stopped_;							// 0x0130
	private:
		char pad_0131[0x0007];					// 0x0131
	public:
		lua_CFunction gc_policy_;				// 0x0138
		hksSize pause_trigger_memory_usage_;	// 0x0140
		hksInt32 step_trigger_countdown_;		// 0x0148
		hksUint32 string_table_index_;			// 0x014C
		hksUint32 string_table_size_;			// 0x0150
	private:
		char pad_0154[0x0004];					// 0x0154
	public:
		UserData* last_black_ud_;				// 0x0158
		UserData* active_ud_;					// 0x0160
	};
	ENGINE_ASSERT_SZ(GarbageCollector, 0x0168);
}

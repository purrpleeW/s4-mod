#pragma once
#include "engine/engine_common.hpp"

namespace s2::hks {
	class Breakpoint {
	public:
		enum class bpStates {
			NEW = 0,
			DISABLED = 1,
			ENABLED = 2
		};

		enum class expressionTypes {
			LOWER_EXPRESSION_TYPES_BOUND = -2,
			HITCOUNT_EXP_LT = -1,
			HITCOUNT_EXP_EQ = 0,
			HITCOUNT_EXP_GT = 1,
			HITCOUNT_DISABLED = 2,
			UPPER_EXPRESSION_TYES_BOUND = 3
		};

		char file_[512];						// 0x0000
		char expression_[512];					// 0x0200
		Breakpoint* next_;						// 0x0400
		Breakpoint* next_in_chain_;				// 0x0408
		lua_State* coroutine_;					// 0x0410
		hksInt32 target_hit_count_;				// 0x0418
		hksInt32 hit_count_;					// 0x041C
		hksInt32 line_;							// 0x0420
		hksInt32 ref_count_;					// 0x0424
		bpStates bp_state_;						// 0x0428
		expressionTypes hit_count_expression_;	// 0x042C
	};
	ENGINE_ASSERT_SZ(Breakpoint, 0x0430);
}

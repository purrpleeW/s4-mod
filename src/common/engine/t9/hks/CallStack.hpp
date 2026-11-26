#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/hksInstruction.hpp"
#include "engine/t9/HksObject.hpp"

namespace t9::hks {
	class CallStack {
	public:
		class ActivationRecord {
		public:
			HksObject* base_;						// 0x0000
			const hksInstruction* return_address_;	// 0x0008
			hksShort16 tail_call_depth_;			// 0x0010
			hksShort16 num_varargs_;				// 0x0012
			hksInt32 num_expected_returns_;			// 0x0014
		};

		ActivationRecord* records_;					// 0x0000
		ActivationRecord* last_record_;				// 0x0008
		ActivationRecord* current_;					// 0x0010
		const hksInstruction* current_lua_pc_;		// 0x0018
		const hksInstruction* hook_return_addr_;	// 0x0020
		hksInt32 hook_level_;						// 0x0028
	private:
		char pad_002C[0x0004];						// 0x002C
	};
	ENGINE_ASSERT_SZ(CallStack, 0x0030);
}

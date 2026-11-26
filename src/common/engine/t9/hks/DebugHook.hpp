#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/hksInstruction.hpp"

namespace t9::hks {
	class DebugHook {
	public:
		lua_Hook callback_;				// 0x0000
		hksInt32 mask_;					// 0x0008
		hksInt32 count_;				// 0x000C
		hksInt32 counter_;				// 0x0010
		bool in_use_;					// 0x0014
	private:
		char pad_0015[0x0003];			// 0x0015
	public:
		const hksInstruction* prev_pc_;	// 0x0018
	};
	ENGINE_ASSERT_SZ(DebugHook, 0x0020);
}

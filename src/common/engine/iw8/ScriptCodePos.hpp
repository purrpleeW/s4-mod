#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	union ScriptCodePos {
		const char* script_pos_;		// 0x0000
		std::uintptr_t generic_pos_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(ScriptCodePos, 0x0008);
}

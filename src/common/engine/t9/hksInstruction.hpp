#pragma once
#include "engine/engine_common.hpp"

namespace t9 {
	class hksInstruction {
	public:
		hksBytecodeInstruction code_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(hksInstruction, 0x0004);
}

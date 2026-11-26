#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class BCInsLine {
	public:
		BCIns ins_;		// 0x0000
		BCLine line_;	// 0x0004
	};
	ENGINE_ASSERT_SZ(BCInsLine, 0x0008);
}

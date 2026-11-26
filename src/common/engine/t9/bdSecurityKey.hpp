#pragma once
#include "engine/engine_common.hpp"

namespace t9 {
	class bdSecurityKey {
	public:
		bdUByte8 ab_[16];	// 0x0000
	};
	ENGINE_ASSERT_SZ(bdSecurityKey, 0x0010);
}

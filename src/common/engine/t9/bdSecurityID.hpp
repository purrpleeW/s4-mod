#pragma once
#include "engine/engine_common.hpp"

namespace t9 {
	class bdSecurityID {
	public:
		bdUByte8 ab_[8];	// 0x0000
	};
	ENGINE_ASSERT_SZ(bdSecurityID, 0x0008);
}

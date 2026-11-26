#pragma once
#include "engine/engine_common.hpp"

namespace iw9 {
	class bdSecurityKey {
	public:
		std::uint8_t ab_[16];	// 0x0000
	};
	ENGINE_ASSERT_SZ(bdSecurityKey, 0x0010);
}

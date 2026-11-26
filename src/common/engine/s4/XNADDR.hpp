#pragma once
#include "engine/engine_common.hpp"

namespace s4 {
	class XNADDR {
	public:
		std::uint8_t addr_buff_[0x54];	// 0x0000
	};
	ENGINE_ASSERT_SZ(XNADDR, 0x0054);
}

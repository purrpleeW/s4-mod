#pragma once
#include "engine/engine_common.hpp"

namespace t9 {
	class SerializedAdr {
	public:
		std::uint8_t valid_;			// 0x0000
		std::uint8_t addr_buff_[0x43];	// 0x0001
	};
	ENGINE_ASSERT_SZ(SerializedAdr, 0x0044);
}

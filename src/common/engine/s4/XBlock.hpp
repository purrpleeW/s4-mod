#pragma once
#include "engine/engine_common.hpp"

namespace s4 {
	class XBlock {
	public:
		std::uint8_t* data_;	// 0x0000
		std::uint64_t size_;	// 0x0008
	};
	ENGINE_ASSERT_SZ(XBlock, 0x0010);
}

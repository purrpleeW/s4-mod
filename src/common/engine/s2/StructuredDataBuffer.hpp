#pragma once
#include "engine/engine_common.hpp"

namespace s2 {
	class StructuredDataBuffer {
	public:
		std::uint8_t* data_;	// 0x0000
		std::uint32_t size_;	// 0x0008
	private:
		char pad_000C[0x0004];	// 0x000C
	};
	ENGINE_ASSERT_SZ(StructuredDataBuffer, 0x0010);
}

#pragma once
#include "engine/engine_common.hpp"

namespace sat {
	class GscObj {
	public:
		XHash hash_;			// 0x0000
		std::uint32_t size_;	// 0x0008
	private:
		char pad_000C[0x0004];	// 0x000C
	public:
		std::uint8_t* buffer_;	// 0x0010
	};
	ENGINE_ASSERT_SZ(GscObj, 0x0018);
}

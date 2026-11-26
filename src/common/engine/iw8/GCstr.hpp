#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/GCRef.hpp"

namespace iw8 {
	class GCstr {
	public:
		GCRef next_gc_;			// 0x0000
		std::uint8_t marked_;	// 0x0008
		std::uint8_t gct_;		// 0x0009
		std::uint8_t reserved_;	// 0x000A
		std::uint8_t unused_;	// 0x000B
		std::uint32_t hash_;	// 0x000C
		std::uint32_t len_;		// 0x0010
	private:
		char pad_0014[0x0004];	// 0x0014
	};
	ENGINE_ASSERT_SZ(GCstr, 0x0018);
}

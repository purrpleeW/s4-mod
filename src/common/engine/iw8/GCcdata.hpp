#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/GCRef.hpp"

namespace iw8 {
	class GCcdata {
	public:
		GCRef next_gc_;				// 0x0000
		std::uint8_t marked_;		// 0x0008
		std::uint8_t gct_;			// 0x0009
		std::uint16_t c_type_id;	// 0x000A
	private:
		char pad_000C[0x0004];		// 0x000C
	};
	ENGINE_ASSERT_SZ(GCcdata, 0x0010);
}

#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/GCRef.hpp"

namespace iw8 {
	class GCudata {
	public:
		GCRef next_gc_;			// 0x0000
		std::uint8_t marked_;	// 0x0008
		std::uint8_t gct_;		// 0x0009
		std::uint8_t ud_type_;	// 0x000A
		std::uint8_t unused_2_;	// 0x000B
	private:
		char pad_000C[0x0004];	// 0x000C
	public:
		GCRef env_;				// 0x0010
		std::uint32_t len_;		// 0x0018
	private:
		char pad_001C[0x0004];	// 0x001C
	public:
		GCRef metatable_;		// 0x0020
		std::uint32_t align_1_;	// 0x0028
	private:
		char pad_002C[0x0004];	// 0x002C
	};
	ENGINE_ASSERT_SZ(GCudata, 0x0030);
}

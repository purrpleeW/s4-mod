#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/GCRef.hpp"
#include "engine/iw8/MRef.hpp"
#include "engine/iw8/TValue.hpp"

namespace iw8 {
	class GCupval {
	public:
		GCRef next_gc_;				// 0x0000
		std::uint8_t marked_;		// 0x0008
		std::uint8_t gct_;			// 0x0009
		std::uint8_t closed_;		// 0x000A
		std::uint8_t immutable_;	// 0x000B
	private:
		char pad_000C[0x0004];		// 0x000C
	public:
		union {
			TValue tv_;				// 0x0010
			GCRef prev_;			// 0x0010
		};
		GCRef next_;				// 0x0018
		MRef v_;					// 0x0020
		std::uint32_t d_hash_;		// 0x0028
	private:
		char pad_002C[0x0004];		// 0x002C
	};
	ENGINE_ASSERT_SZ(GCupval, 0x0030);
}

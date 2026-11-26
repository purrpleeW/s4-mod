#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/GCRef.hpp"
#include "engine/iw8/MRef.hpp"

namespace iw8 {
	class GCtab {
	public:
		GCRef next_gc_;			// 0x0000
		std::uint8_t marked_;	// 0x0008
		std::uint8_t gct_;		// 0x0009
		std::uint8_t no_mm_;	// 0x000A
		char colo_;				// 0x000B
	private:
		char pad_000C[0x0004];	// 0x000C
	public:
		MRef array_;			// 0x0010
		GCRef gc_list;			// 0x0018
		GCRef metatable_;		// 0x0020
		MRef node_;				// 0x0028
		std::uint32_t a_size_;	// 0x0030
		std::uint32_t h_mask_;	// 0x0034
		MRef free_top_;			// 0x0038
	};
	ENGINE_ASSERT_SZ(GCtab, 0x0040);
}

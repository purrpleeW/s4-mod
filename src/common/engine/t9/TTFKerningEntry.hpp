#pragma once
#include "engine/engine_common.hpp"

namespace t9 {
	struct TTFKerningEntry {
		std::uint16_t height_;			// 0x0000
		std::uint16_t prev_char_index_;	// 0x0002
		std::uint16_t next_char_index_;	// 0x0004
		std::int16_t kerning_;			// 0x0006
	};
	ENGINE_ASSERT_SZ(TTFKerningEntry, 0x0008);
}

#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class FxGlassCrackHeader {
	public:
		std::uint16_t unique_vert_count_;	// 0x0000
		std::uint8_t begin_vert_index_;		// 0x0002
		std::uint8_t end_vert_index_;		// 0x0003
	};
	ENGINE_ASSERT_SZ(FxGlassCrackHeader, 0x0004);
}

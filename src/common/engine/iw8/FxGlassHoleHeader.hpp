#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class FxGlassHoleHeader {
	public:
		std::uint16_t unique_vert_count_;	// 0x0000
		std::uint8_t touch_vert_;			// 0x0002
	private:
		char pad_0003[0x0001];				// 0x0003
	};
	ENGINE_ASSERT_SZ(FxGlassHoleHeader, 0x0004);
}

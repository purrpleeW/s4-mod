#pragma once
#include "engine/engine_common.hpp"

namespace jup {
	union GfxColor {
		std::uint8_t array_[4];	// 0x0000
		std::uint32_t packed_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(GfxColor, 0x0004);
}

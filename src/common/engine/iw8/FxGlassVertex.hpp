#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class FxGlassVertex {
	public:
		std::int16_t x_;	// 0x0000
		std::int16_t y_;	// 0x0002
	};
	ENGINE_ASSERT_SZ(FxGlassVertex, 0x0004);
}

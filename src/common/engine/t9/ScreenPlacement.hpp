#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/vec2_t.hpp"

namespace t9 {
	class ScreenPlacement {
	private:
		char pad_0000[0x0088];		// 0x0000
	public:
		vec2_t real_viewport_size_;	// 0x0088
	private:
		char pad_0090[0x005C];		// 0x0090
	};
	ENGINE_ASSERT_SZ(ScreenPlacement, 0x00EC);
}

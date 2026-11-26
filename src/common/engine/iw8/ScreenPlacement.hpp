#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/vec2_t.hpp"

namespace iw8 {
	class ScreenPlacement {
	public:
		vec2_t scale_virtual_to_real_;	// 0x0000
		vec2_t scale_virtual_to_full_;	// 0x0008
		vec2_t scale_real_to_virtual_;	// 0x0010
		vec2_t real_viewport_position_;	// 0x0018
		vec2_t real_viewport_size_;		// 0x0020
		vec2_t virtual_viewable_min_;	// 0x0028
		vec2_t virtual_viewable_max_;	// 0x0030
		vec2_t real_viewable_min_;		// 0x0038
		vec2_t real_viewable_max_;		// 0x0040
		vec2_t virtual_adjustable_min_;	// 0x0048
		vec2_t virtual_adjustable_max_;	// 0x0050
		vec2_t real_adjustable_min_;	// 0x0058
		vec2_t real_adjustable_max_;	// 0x0060
		float sub_screen_left_;			// 0x0068
	};
	ENGINE_ASSERT_SZ(ScreenPlacement, 0x006C);
}

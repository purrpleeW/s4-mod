#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/enums/FontCacheFXFlags.hpp"

namespace t9 {
	class Font {
	public:
		const char* font_name_;		// 0x0000
		TTFDef* ttf_;				// 0x0008
		int pixel_height_;			// 0x0010
		FontCacheFXFlags fx_flags_;	// 0x0014
		MaterialHandle material_;	// 0x0018
	};
	ENGINE_ASSERT_SZ(Font, 0x0020);
}
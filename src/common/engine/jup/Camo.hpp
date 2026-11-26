#pragma once
#include "engine/engine_common.hpp"
#include "engine/jup/GfxImage.hpp"
#include "engine/jup/enums/scr_string_t.hpp"

namespace jup {
	class Camo {
	public:
		XHash name_hash_;					// 0x0000
		scr_string_t internal_name_;		// 0x0008
	private:
		char pad_000C[0x0004];				// 0x000C
	public:
		GfxImage* primary_images_[3];		// 0x0010
		GfxImage* secondary_images_[3];		// 0x0028
		GfxImage* tertiary_images_[3];		// 0x0040
		GfxImage* quaternary_images_[3];	// 0x0058
		GfxImage* overlay_image_;			// 0x0070
	private:
		char pad_0078[0x0158];
	};
	ENGINE_ASSERT_SZ(Camo, 0x01D0);
}

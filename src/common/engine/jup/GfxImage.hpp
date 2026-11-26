#pragma once
#include "engine/engine_common.hpp"

namespace jup {
	class GfxImage {
	public:
		XHash name_hash_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(GfxImage, 0x0008);
}

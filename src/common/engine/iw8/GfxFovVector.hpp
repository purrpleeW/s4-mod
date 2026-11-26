#include "engine/engine_common.hpp"

namespace iw8 {
	class GfxFovVector {
	public:
		float tan_half_fov_x_;	// 0x0000
		float tan_half_fov_y_;	// 0x0004
	};
	ENGINE_ASSERT_SZ(GfxFovVector, 0x0008);
}

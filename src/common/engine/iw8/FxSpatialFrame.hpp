#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/vec4_t.hpp"

namespace iw8 {
	class FxSpatialFrame {
	public:
		vec4_t quat_;	// 0x0000
		vec3_t origin_;	// 0x0010
	};
	ENGINE_ASSERT_SZ(FxSpatialFrame, 0x001C);
}

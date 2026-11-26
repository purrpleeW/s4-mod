#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class CustomizationModels {
	public:
		int head_model_index_;	// 0x0000
		int body_model_index_;	// 0x0004
		int execution_id_;		// 0x0008
	};
	ENGINE_ASSERT_SZ(CustomizationModels, 0x000C);
}

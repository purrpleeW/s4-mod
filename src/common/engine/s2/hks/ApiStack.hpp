#pragma once
#include "engine/engine_common.hpp"
#include "engine/s2/HksObject.hpp"

namespace s2::hks {
	struct ApiStack {
		HksObject* top_;		// 0x0000
		HksObject* base_;		// 0x0008
		HksObject* alloc_top_;	// 0x0010
		HksObject* bottom_;		// 0x0018
	};
	ENGINE_ASSERT_SZ(ApiStack, 0x0020);
}

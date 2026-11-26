#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class pinglist_t {
	public:
		char adr_str_[64];	// 0x0000
		int start_;			// 0x0040
	};
	ENGINE_ASSERT_SZ(pinglist_t, 0x0044);
}

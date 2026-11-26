#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class bdStopwatch {
	public:
		std::uint64_t start_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(bdStopwatch, 0x0008);
}

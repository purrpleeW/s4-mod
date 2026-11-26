#pragma once
#include "engine/engine_common.hpp"
#include "engine/s4/XBlock.hpp"

namespace s4 {
	class XArchiveBlocks {
	public:
		XBlock blocks_[11];
	};
	ENGINE_ASSERT_SZ(XArchiveBlocks, 0x00B0);
}

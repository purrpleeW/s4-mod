#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/XBlock.hpp"

namespace iw8 {
	class XArchiveBlocks {
	public:
		XBlock blocks_[11];
	};
	ENGINE_ASSERT_SZ(XArchiveBlocks, 0x00B0);
}

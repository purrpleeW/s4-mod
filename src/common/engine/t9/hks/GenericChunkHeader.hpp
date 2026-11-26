#pragma once
#include "engine/engine_common.hpp"

namespace t9::hks {
	class GenericChunkHeader {
	public:
		hksSize flags_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(GenericChunkHeader, 0x0008);
}

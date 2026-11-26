#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/hks/ChunkHeader.hpp"

namespace t9::hks {
	class ChunkList {
	public:
		ChunkHeader head_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(ChunkList, 0x0010);
}

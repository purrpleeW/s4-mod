#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/hks/GenericChunkHeader.hpp"

namespace t9::hks {
	class ChunkHeader : public GenericChunkHeader {
	public:
		ChunkHeader* next_;	// 0x0008
	};
	ENGINE_ASSERT_SZ(ChunkHeader, 0x0010);
}

#pragma once
#include "engine/engine_common.hpp"
#include "engine/s2/hks/GenericChunkHeader.hpp"

namespace s2::hks {
	class ChunkHeader : public GenericChunkHeader {
	public:
		ChunkHeader* next_;	// 0x0008
	};
	ENGINE_ASSERT_SZ(ChunkHeader, 0x0010);
}

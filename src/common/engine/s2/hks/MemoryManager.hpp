#pragma once
#include "engine/engine_common.hpp"
#include "engine/s2/hks/ChunkHeader.hpp"
#include "engine/s2/hks/ChunkList.hpp"

namespace s2::hks {
	class MemoryManager {
	public:
		enum class ChunkColor {
			WHITE = 0,
			BLACK = 1
		};

		lua_Alloc allocator_;			// 0x0000
		void* allocator_ud_;			// 0x0008
		ChunkColor chunk_color_;		// 0x0010
	private:
		char pad_0014[0x0004];			// 0x0014
	public:
		hksSize used_;					// 0x0018
		hksSize high_watermark_;		// 0x0020
		ChunkList allocation_list_;		// 0x0028
		ChunkList sweep_list_;			// 0x0038
		ChunkHeader* last_kept_chunk_;	// 0x0048
		lua_State* state_;				// 0x0050
	};
	ENGINE_ASSERT_SZ(MemoryManager, 0x0058);
}

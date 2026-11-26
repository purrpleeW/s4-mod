#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class StringTable {
	public:
		const char* name_;				// 0x0000
		int column_count_;				// 0x0008
		int row_count_;					// 0x000C
		int unique_cell_count_;			// 0x0010
	private:
		char pad_0014[0x0004];			// 0x0014
	public:
		std::uint16_t* cell_indices_;	// 0x0018
		int* hashes_;					// 0x0020
		const char** strings_;			// 0x0028
	};
	ENGINE_ASSERT_SZ(StringTable, 0x0030);
}

#pragma once
#include "engine/engine_common.hpp"
#include "engine/s2/StringTableCell.hpp"

namespace s2 {
	class StringTable {
	public:
		const char* name_;			// 0x0000
		int column_count_;			// 0x0008
		int row_count_;				// 0x000C
		StringTableCell* values_;	// 0x0010
	};
	ENGINE_ASSERT_SZ(StringTable, 0x0018);
}

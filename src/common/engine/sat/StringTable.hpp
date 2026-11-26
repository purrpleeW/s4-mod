#pragma once
#include "engine/engine_common.hpp"
#include "engine/sat/StringTableColumn.hpp"

namespace sat {
	class StringTable {
	public:
		XHash hash_;					// 0x0000
		int column_count_;				// 0x0008
		int row_count_;					// 0x000C
		int unique_cell_count_;			// 0x0010
	private:
		char pad_0014[0x0004];			// 0x0014
	public:
		StringTableColumn* columns_;	// 0x0018
	private:
		char pad_0020[0x0008];			// 0x0020
	};
	ENGINE_ASSERT_SZ(StringTable, 0x0028);
}

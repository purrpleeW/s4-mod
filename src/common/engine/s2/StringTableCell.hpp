#pragma once
#include "engine/engine_common.hpp"

namespace s2 {
	class StringTableCell {
	public:
		const char* string_;	// 0x0000
		int hash_;				// 0x0008
	private:
		char pad_000C[0x0004];	// 0x000C
	};
	ENGINE_ASSERT_SZ(StringTableCell, 0x0010);
}

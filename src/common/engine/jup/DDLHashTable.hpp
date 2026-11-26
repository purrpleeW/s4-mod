#pragma once
#include "engine/engine_common.hpp"
#include "engine/jup/DDLHash.hpp"

namespace jup {
	class DDLHashTable {
	public:
		DDLHash* list_;	// 0x0000
		int count_;		// 0x0008
		int max_;		// 0x000C
	};
	ENGINE_ASSERT_SZ(DDLHashTable, 0x0010);
}

#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/DDLHash.hpp"

namespace iw8 {
	class DDLHashTable {
	public:
		DDLHash* list_;	// 0x0000
		int count_;		// 0x0008
		int max_;		// 0x000C
	};
	ENGINE_ASSERT_SZ(DDLHashTable, 0x0010);
}

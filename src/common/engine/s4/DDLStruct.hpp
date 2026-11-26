#pragma once
#include "engine/engine_common.hpp"
#include "engine/s4/DDLHashTable.hpp"
#include "engine/s4/DDLMember.hpp"

namespace s4 {
	class DDLStruct {
	public:
		const char* name_;				// 0x0000
		int bit_size_;					// 0x0008
		int member_count_;				// 0x000C
		DDLMember* members_;			// 0x0010
		DDLHashTable hash_table_upper_;	// 0x0018
		DDLHashTable hash_table_lower_;	// 0x0028
	};
	ENGINE_ASSERT_SZ(DDLStruct, 0x0038);
}

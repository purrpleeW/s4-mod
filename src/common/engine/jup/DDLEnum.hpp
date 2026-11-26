#pragma once
#include "engine/engine_common.hpp"
#include "engine/jup/DDLHashTable.hpp"

namespace jup {
	class DDLEnum {
	public:
		const char* name_;			// 0x0000
		int member_count_;			// 0x0008
	private:
		char pad_000C[0x0004];		// 0x000C
	public:
		const char** members_;		// 0x0010
		DDLHashTable hash_table_;	// 0x0018
	};
	ENGINE_ASSERT_SZ(DDLEnum, 0x0028);
}

#pragma once
#include "engine/engine_common.hpp"

namespace iw9 {
	class DDLMember {
	public:
		const char* name_;			// 0x0000
		int index_;					// 0x0008
		int bit_size_;				// 0x000C
		int limit_size_;			// 0x0010
		int offset_;				// 0x0014
		int type_;					// 0x0018
		int external_index_;		// 0x001C
		std::uint32_t range_limit_;	// 0x0020
		bool is_array_;				// 0x0024
	private:
		char pad_0025[0x0003];		// 0x0025
	public:
		int array_size_;			// 0x0028
		int enum_index_;			// 0x002C
	};
	ENGINE_ASSERT_SZ(DDLMember, 0x0030);
}

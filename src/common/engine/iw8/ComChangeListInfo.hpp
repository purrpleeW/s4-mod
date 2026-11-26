#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class ComChangeListInfo {
	public:
		std::uint32_t change_list_number_;	// 0x0000
		int time_;							// 0x0004
		const char* user_name_;				// 0x0008
	};
	ENGINE_ASSERT_SZ(ComChangeListInfo, 0x0010);
}

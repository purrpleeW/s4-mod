#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class SndAliasList {
	public:
		const char* alias_name_;	// 0x0000
		std::uint32_t id_;			// 0x0008
	private:
		char pad_000C[0x0004];		// 0x000C
	public:
		SndAlias* head_;			// 0x0010
		int count_;					// 0x0018
		int sequence_;				// 0x001C
	};
	ENGINE_ASSERT_SZ(SndAliasList, 0x0020);
}

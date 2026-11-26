#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/XUID.hpp"

namespace iw8 {
	class DWFriend {
	public:
		XUID xuid_;					// 0x0000
		char name_[0x40];			// 0x0008
		char name_with_hash_[0x40];	// 0x0048
		bool is_platform_friend_;	// 0x0088
	private:
		char pad_0089[0x0007];		// 0x0089
	};
	ENGINE_ASSERT_SZ(DWFriend, 0x0090);
}

#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class OnlineChatId {
		std::uint64_t id_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(OnlineChatId, 0x0008);
}

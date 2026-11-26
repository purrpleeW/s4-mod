#pragma once
#include "engine/engine_common.hpp"

namespace iw9 {
	class PlayerSocialDataHandle {
	public:
		std::uint16_t index_;		// 0x0000
		std::uint16_t identifier_;	// 0x0002
	};
	ENGINE_ASSERT_SZ(PlayerSocialDataHandle, 0x0004);
}

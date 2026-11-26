#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/SessionDynamicData.hpp"

namespace iw8 {
	class SessionData {
	public:
		int controller_index_;		// 0x0000
		SessionDynamicData dyn_;	// 0x0004
	};
	ENGINE_ASSERT_SZ(SessionData, 0x0070);
}

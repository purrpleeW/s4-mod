#pragma once
#include "engine/engine_common.hpp"
#include "engine/jup/SessionDynamicData.hpp"

namespace jup {
	class SessionData {
	public:
		SessionDynamicData dyn_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(SessionData, 0x006C);
}

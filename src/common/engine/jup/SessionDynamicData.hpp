#pragma once
#include "engine/engine_common.hpp"
#include "engine/jup/XSESSION_INFO.hpp"

namespace jup {
	class SessionDynamicData {
	public:
		XSESSION_INFO session_info_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(SessionDynamicData, 0x006C);
}

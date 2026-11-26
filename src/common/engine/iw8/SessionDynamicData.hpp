#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/XSESSION_INFO.hpp"

namespace iw8 {
	class SessionDynamicData {
	public:
		XSESSION_INFO session_info_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(SessionDynamicData, 0x006C);
}

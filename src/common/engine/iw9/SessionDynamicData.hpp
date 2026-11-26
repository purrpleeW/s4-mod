#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw9/XSESSION_INFO.hpp"

namespace iw9 {
	class SessionDynamicData {
	public:
		XSESSION_INFO session_info_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(SessionDynamicData, 0x006C);
}

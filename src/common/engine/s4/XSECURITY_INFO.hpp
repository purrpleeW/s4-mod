#pragma once
#include "engine/engine_common.hpp"
#include "engine/s4/bdSecurityID.hpp"
#include "engine/s4/bdSecurityKey.hpp"

namespace s4 {
	class XSECURITY_INFO {
	public:
		bdSecurityID id_;	// 0x0000
		bdSecurityKey key_;	// 0x0008
	};
	ENGINE_ASSERT_SZ(XSECURITY_INFO, 0x0018);
}

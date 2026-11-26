#pragma once
#include "engine/engine_common.hpp"
#include "engine/jup/bdSecurityID.hpp"
#include "engine/jup/bdSecurityKey.hpp"

namespace jup {
	class XSECURITY_INFO {
	public:
		bdSecurityID id_;	// 0x0000
		bdSecurityKey key_;	// 0x0008
	};
	ENGINE_ASSERT_SZ(XSECURITY_INFO, 0x0018);
}

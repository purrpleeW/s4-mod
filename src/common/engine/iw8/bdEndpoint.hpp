#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/bdCommonAddr.hpp"
#include "engine/iw8/bdReference.hpp"
#include "engine/iw8/bdSecurityID.hpp"

namespace iw8 {
	class bdEndpoint {
	public:
		bdReference<bdCommonAddr> ca_;	// 0x0000
		bdSecurityID sec_id_;			// 0x0008
	};
	ENGINE_ASSERT_SZ(bdEndpoint, 0x0010);
}

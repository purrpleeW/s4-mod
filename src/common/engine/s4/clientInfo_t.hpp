#pragma once
#include "engine/engine_common.hpp"

namespace s4 {
	class clientInfo_t {
	public:
		char pad_0000[0x00D0];	// 0x00D0
	};
	ENGINE_ASSERT_SZ(clientInfo_t, 0x00D0);
}

#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class bdString {
	public:
		char* string_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(bdString, 0x0008);
}

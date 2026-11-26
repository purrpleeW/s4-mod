#pragma once
#include "engine/engine_common.hpp"

namespace s4 {
	class BattleNetString {
	public:
		char* data_;		// 0x0000
		std::size_t size_;	// 0x0008
	};
	ENGINE_ASSERT_SZ(BattleNetString, 0x0010);
}

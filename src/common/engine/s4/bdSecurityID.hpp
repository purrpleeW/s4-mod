#pragma once
#include "engine/engine_common.hpp"

namespace s4 {
	class bdSecurityID {
	public:
		std::uint8_t ab_[8];	// 0x0000

		bool is_valid() {
			return DEREF_PTR_AS(std::uintptr_t, &this->ab_[0]) != 0;
		}
	};
	ENGINE_ASSERT_SZ(bdSecurityID, 0x0008);
}

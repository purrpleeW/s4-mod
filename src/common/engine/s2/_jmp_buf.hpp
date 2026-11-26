#pragma once
#include "engine/engine_common.hpp"

namespace s2 {
	class _jmp_buf {
	public:
		std::int64_t jb_[12];	// 0x0000
	};
	ENGINE_ASSERT_SZ(_jmp_buf, 0x0060);
}

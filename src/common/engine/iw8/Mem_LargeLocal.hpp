#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class Mem_LargeLocal {
	public:
		void* ptr_;			// 0x0000
		std::size_t size_;	// 0x0008
	};
	ENGINE_ASSERT_SZ(Mem_LargeLocal, 0x0010);
}

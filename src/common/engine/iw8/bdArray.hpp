#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	template <typename T>
	class bdArray {
	public:
		T* data_;					// 0x0000
		std::uint32_t capacity_;	// 0x0008
		std::uint32_t size_;		// 0x000C
	};
	ENGINE_ASSERT_SZ(bdArray<void>, 0x0010);
}

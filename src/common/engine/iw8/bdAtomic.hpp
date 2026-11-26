#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	template <typename T>
	class bdAtomic {
	public:
		std::atomic<T> value_;	// 0x0000
	};
}

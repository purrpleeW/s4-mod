#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	template <typename T, std::size_t N>
	class bdFixedArray {
	public:
		T array_[N];
		std::uint32_t size_;
	};
}

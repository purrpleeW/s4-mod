#pragma once
#include "engine/engine_common.hpp"

namespace ntl::internal {
	template <typename T, std::size_t Count, std::size_t Align = alignof(T)>
	struct fixed_memory_block {
		alignas(Align) std::uint8_t buffer_[sizeof(T) * Count];
	};
}

#pragma once
#include "engine/engine_common.hpp"
#include "engine/ntl/internal/fixed_memory_block.hpp"
#include "engine/ntl/internal/pool_allocator_impl.hpp"

namespace ntl {
	template <typename T, std::size_t Count, std::size_t Align = alignof(T)>
	class fixed_pool_allocator : public internal::pool_allocator_impl<internal::fixed_memory_block<T, Count, Align>, sizeof(T)> {
	public:
		using base_type = internal::pool_allocator_impl<internal::fixed_memory_block<T, Count, Align>, sizeof(T)>;
		using base_type::base_type;
	};
}

#pragma once
#include "engine/engine_common.hpp"
#include "engine/ntl/internal/pool_allocator_freelist.hpp"

namespace ntl::internal {
	template <typename DataType, std::size_t ObjectSize>
	class pool_allocator_impl {
	public:
		DataType data_;
		pool_allocator_freelist<ObjectSize> free_list_;

		bool owns_ptr(const void* p) const {
			if (!p) {
				return false;
			}
			auto begin = PTR_AS(const std::uintptr_t, &this->data_);
			auto end = begin + sizeof(this->data_);
			auto ptr = PTR_AS(const std::uintptr_t, p);
			if (ptr < begin || ptr >= end) {
				return false;
			}
			return ((ptr - begin) % ObjectSize) == 0;
		}
	};
}

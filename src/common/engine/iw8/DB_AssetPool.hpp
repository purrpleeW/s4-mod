#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class DB_AssetPool {
	public:
		void* entries_;
		void* free_head_;
		std::uint32_t pool_size_;
		std::uint32_t element_size_;

		template <typename T>
		T* get_entries() {
			return (T*)this->entries_;
		}

		template <typename T>
		T* get_entry(std::size_t index) {
			return PTR_AS(T*, PTR_AS(std::uintptr_t, this->entries_) + (index * this->element_size_));
		}

		bool is_element_present(void* element) {
			return PTR_AS(std::uintptr_t, element) >= PTR_AS(std::uintptr_t, this->entries_)
				&& PTR_AS(std::uintptr_t, element) <= (PTR_AS(std::uintptr_t, this->entries_) + (this->pool_size_ * this->element_size_));
		}

		std::size_t get_entry_count() {
			auto distance = PTR_AS(std::uintptr_t, this->free_head_) - PTR_AS(std::uintptr_t, this->entries_);
			return distance / VAL_AS(std::size_t, this->element_size_);
		}
	};
	ENGINE_ASSERT_SZ(DB_AssetPool, 0x0018);
}

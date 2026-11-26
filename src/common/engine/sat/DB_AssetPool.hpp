#pragma once
#include "engine/engine_common.hpp"

namespace sat {
	/*
		Was of size 0x00B8 before Season 4 (1.10.0)
	*/
	class DB_AssetPool {
	public:
		void* entries_;						// 0x0000
		void* free_head_;					// 0x0008
		std::uint32_t pool_size_;			// 0x0010
		std::uint32_t element_size_;		// 0x0014
		std::uint32_t loaded_pool_size_;	// 0x0018
	private:
		char pad_001C[0x00AC];				// 0x001C
	public:
		template <typename T>
		T* get_entries() {
			return (T*)this->entries_;
		}
	};
	ENGINE_ASSERT_SZ(DB_AssetPool, 0x00C8);
}

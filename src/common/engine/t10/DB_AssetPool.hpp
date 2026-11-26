#pragma once
#include "engine/engine_common.hpp"

namespace t10 {
	class DB_AssetPool {
	public:
		void* entries_;						// 0x0000
		void* free_head_;					// 0x0008
		std::uint32_t pool_size_;			// 0x0010
		std::uint32_t element_size_;		// 0x0014
		std::uint32_t loaded_pool_size_;	// 0x0018
	private:
		char pad_001C[0x008C];				// 0x001C
	public:
		template <typename T>
		T* get_entries() {
			return (T*)this->entries_;
		}
	};
	ENGINE_ASSERT_SZ(DB_AssetPool, 0x00A8);
}

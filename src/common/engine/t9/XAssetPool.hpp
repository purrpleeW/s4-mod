#pragma once
#include "engine/engine_common.hpp"

namespace t9 {
	class XAssetPool {
	public:
		void* pool_;				// 0x0000
		std::uint32_t item_size_;	// 0x0008
		int item_count_;			// 0x000C
		bool is_singleton_;			// 0x0010
	private:
		char pad_0011[0x0003];		// 0x0011
	public:
		int item_alloc_count_;		// 0x0014
		void* free_head_;			// 0x0018
	};
	ENGINE_ASSERT_SZ(XAssetPool, 0x0020);
}

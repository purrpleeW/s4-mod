#pragma once
#include "engine/engine_common.hpp"

namespace ntl::internal {
	class pool_allocator_pointer_freelist {
	public:
		class free_item_pointer {
		public:
			free_item_pointer* next_;	// 0x0000
		};
		free_item_pointer head_;		// 0x0000
	};
	ENGINE_ASSERT_SZ(pool_allocator_pointer_freelist::free_item_pointer, 0x0008);
	ENGINE_ASSERT_SZ(pool_allocator_pointer_freelist, 0x0008);
}

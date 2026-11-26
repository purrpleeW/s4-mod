#pragma once
#include "engine/engine_common.hpp"
#include "engine/ntl/internal/pool_allocator_pointer_freelist.hpp"

namespace ntl::internal {
	template <std::size_t ObjectSize>
	class pool_allocator_freelist : public pool_allocator_pointer_freelist {
	public:
		// no methods
	};
	ENGINE_ASSERT_SZ(pool_allocator_freelist<0>, 0x0008);
}

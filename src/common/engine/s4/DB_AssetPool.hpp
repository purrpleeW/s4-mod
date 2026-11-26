#pragma once
#include "engine/engine_common.hpp"

namespace s4 {
	class DB_AssetPool {
	public:
		void* entries_;
		void* free_head_;
		std::uint32_t pool_size_;
		std::uint32_t element_size_;
	};
	ENGINE_ASSERT_SZ(DB_AssetPool, 0x0018);
}

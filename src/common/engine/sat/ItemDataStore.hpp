#pragma once
#include "engine/engine_common.hpp"

namespace sat {
	class ItemDataStore {
	public:
		std::uint32_t* unlocked_ids_;	// 0x0000
	private:
		char pad_0008[0x0004];			// 0x0008
	public:
		std::uint32_t max_ids_;			// 0x000C
	};
	ENGINE_ASSERT_SZ(ItemDataStore, 0x0010);
}

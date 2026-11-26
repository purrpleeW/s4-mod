#pragma once
#include "engine/engine_common.hpp"

namespace sat {
#	pragma pack(push, 1)
	class LootItem {
	public:
		std::uint32_t item_id_;			// 0x0000
		std::uint64_t item_quantity_;	// 0x0004
	};
#	pragma pack(pop)
	ENGINE_ASSERT_SZ(LootItem, 0x000C);
}

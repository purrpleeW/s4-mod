#pragma once
#include "engine/engine_common.hpp"
#include "engine/sat/ItemDataStore.hpp"
#include "engine/sat/LootItem.hpp"

namespace sat {
	class LootItemContainer {
	public:
		std::uint64_t unk_0_;			// 0x0000
		LootItem* loot_items_;			// 0x0008
	private:
		char pad_0010[0x0010];			// 0x0010
	public:
		ItemDataStore data_stores_[1];	// 0x0020
	};
	ENGINE_ASSERT_SZ(LootItemContainer, 0x0030);
}

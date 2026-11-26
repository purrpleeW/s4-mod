#pragma once
#include "engine/engine_common.hpp"
#include "engine/sat/LootItemContainer.hpp"

namespace sat {
	class LootItemContainerStore {
	private:
		char pad_0000[0x0088];				// 0x0000
	public:
		LootItemContainer containers_[1];	// 0x0008
	};
	ENGINE_ASSERT_SZ(LootItemContainerStore, 0x00B8);
}

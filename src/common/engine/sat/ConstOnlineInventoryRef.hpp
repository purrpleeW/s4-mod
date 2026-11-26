#pragma once
#include "engine/engine_common.hpp"
#include "engine/sat/LootItemContainerStore.hpp"

namespace sat {
	class ConstOnlineInventoryRef {
	public:
		LootItemContainerStore* container_store_;	// 0x0000
	private:
		char pad_0008[0x0008];						// 0x0008
	};
	ENGINE_ASSERT_SZ(ConstOnlineInventoryRef, 0x0010);
}

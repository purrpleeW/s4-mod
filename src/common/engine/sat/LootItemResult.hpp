#pragma once
#include "engine/engine_common.hpp"

namespace sat {
	class LootItemResult {
	public:
		int unk_0_;				// 0x0000
		std::uint16_t count_;	// 0x0004
	private:
		char pad_0006[0x0002];	// 0x0006
	public:
		int unk_1_;				// 0x0008
	};
	ENGINE_ASSERT_SZ(LootItemResult, 0x000C);
}

#pragma once
#include "engine/engine_common.hpp"

namespace jup {
	class PartyStopReason {
	private:
		char pad_0000[0x0008];	// 0x0000
	public:
		std::uint16_t var1_;	// 0x0008
		std::uint8_t var2_;		// 0x000A
	private:
		char pad_000B[0x0001];	// 0x000B
	};
	ENGINE_ASSERT_SZ(PartyStopReason, 0x000C);
}

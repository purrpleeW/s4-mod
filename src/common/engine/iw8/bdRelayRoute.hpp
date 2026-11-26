#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class bdRelayRoute {
		std::uint64_t relay_id_;			// 0x0000
		std::uint32_t relay_routing_id_;	// 0x0008
	private:
		char pad_000C[0x0004];				// 0x000C
	};
	ENGINE_ASSERT_SZ(bdRelayRoute, 0x0010);
}

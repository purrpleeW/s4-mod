#pragma once
#include "engine/engine_common.hpp"
#include "engine/steam/SteamIDComponent_t.hpp"

namespace steam {
	union SteamID_t {
		SteamIDComponent_t comp_;	// 0x0000
		std::uint64_t all_64_bits_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(SteamID_t, 0x0008);
}

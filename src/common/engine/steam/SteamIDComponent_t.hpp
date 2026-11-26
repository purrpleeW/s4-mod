#pragma once
#include "engine/engine_common.hpp"
#include "engine/steam/enums/EAccountType.hpp"
#include "engine/steam/enums/EChatSteamIDInstanceFlags.hpp"
#include "engine/steam/enums/EUniverse.hpp"

namespace steam {
	class SteamIDComponent_t {
	public:
		std::uint32_t account_id_ : 32;						// 0x0000.0
		EChatSteamIDInstanceFlags account_instance_ : 20;	// 0x0004.0
		EAccountType account_type_ : 4;						// 0x0004.20
		EUniverse universe_ : 8;							// 0x0004.28
	};
	ENGINE_ASSERT_SZ(SteamIDComponent_t, 0x0008);
}

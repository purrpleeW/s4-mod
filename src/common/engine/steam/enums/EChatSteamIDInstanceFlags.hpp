#pragma once
#include "engine/engine_common.hpp"

namespace steam {
	enum class EChatSteamIDInstanceFlags {
		k_EChatAccountInstanceMask = 0x00000FFF,
		k_EChatInstanceFlagClan = (0x000FFFFF /* k_unSteamAccountInstanceMask */ + 1) >> 1,
		k_EChatInstanceFlagLobby = (0x000FFFFF /* k_unSteamAccountInstanceMask */ + 1) >> 2,
		k_EChatInstanceFlagMMSLobby = (0x000FFFFF /* k_unSteamAccountInstanceMask */ + 1) >> 3
	};
	DECLARE_BITWISE_OPS(EChatSteamIDInstanceFlags);
}

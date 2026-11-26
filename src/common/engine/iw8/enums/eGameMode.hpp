#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class EGameMode : std::int32_t {
		eFirstGameMode = 0x0,
		eGameModeNone = 0x0,
		eGameModeSinglePlayer = 0x1,
		eGameModeMultiPlayer = 0x2,
		eGameModeCoop = 0x3,
		eLastGameMode = 0x3,
		eFirstErrorMode = 0x4,
		eErrorModeGame = 0x4,
		eErrorModeHandleAtBoot = 0x5,
		eLastErrorMode = 0x5,
	};
}
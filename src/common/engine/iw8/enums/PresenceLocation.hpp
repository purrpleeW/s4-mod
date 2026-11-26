#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class PresenceLocation : std::int8_t {
		LOCATION_MAIN_MENU = 0x0,
		LOCATION_MULTIPLAYER = 0x1,
		LOCATION_SINGLEPLAYER = 0x2,
		LOCATION_LOBBY = 0x3,
		LOCATION_PARTY = 0x4,
		LOCATION_NOT_PLAYING = 0x5,
		LOCATION_THEATRE = 0x9,
		LOCATION_CREDITS = 0xA,
		LOCATION_WARZONE_LOBBY = 0xB,
		LOCATION_WARZONE_PARTY = 0xC,
	};
}
#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class sp_difficulty_t : std::int32_t {
		SP_DIFFICULTY_RECRUIT = 0x0,
		SP_DIFFICULTY_REGULAR = 0x1,
		SP_DIFFICULTY_HARDENED = 0x2,
		SP_DIFFICULTY_VETERAN = 0x3,
		SP_DIFFICULTY_REALISM = 0x4,
		SP_DIFFICULTY_COUNT = 0x5,
	};
}
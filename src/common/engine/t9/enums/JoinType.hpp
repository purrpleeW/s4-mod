#pragma once
#include "engine/engine_common.hpp"

namespace t9 {
	enum class JoinType {
		JOIN_TYPE_NORMAL = 0,
		JOIN_TYPE_PLAYLIST = 1,
		JOIN_TYPE_FRIEND = 2,
		JOIN_TYPE_INVITE = 3,
		JOIN_TYPE_PARTY = 4,
		JOIN_TYPE_GROUPS = 5,
		JOIN_TYPE_COUNT = 6
	};
}

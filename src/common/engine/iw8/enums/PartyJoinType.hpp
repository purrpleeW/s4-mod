#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class PartyJoinType {
		PJT_NONE = 0,
		PJT_PRESENCE = 1,
		PJT_INVITE = 2,
		PJT_HOSTPRESENCE = 3,
		PJT_HOSTREQUEST = 4,
		PJT_MATCHMAKING = 5,
		PJT_HOSTREQUEST_PRIVATE_PARTY = 6,
		PJT_COUNT = 7
	};
}

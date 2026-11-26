#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class PartyProfile_EventType {
		NO_EVENT_TRACK = -1,
		STATE_UPDATE = 0,
		STATE_UPDATE_ACK = 1,
		CLIENTINFO = 2,
		HEARTBEAT = 3,
		HEARTBEAT_ACK = 4,
		NOTPRESENT = 5,
		MATCHMAKINGINFO = 6,
		EVENT_COUNT = 7
	};
}

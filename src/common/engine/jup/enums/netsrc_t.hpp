#pragma once
#include "engine/engine_common.hpp"

namespace jup {
	enum class netsrc_t {
		NS_CLIENT1 = 0,
		NS_CLIENT2 = 1,
		NS_MAXCLIENTS = 2,
		NS_SERVER = 2,
		NS_PACKET = 3,
		NS_BOT_FIRST = 4,
		NS_BOT_LAST = 0x2713,
		NS_INVALID_NETSRC = 0x2714
	};
}

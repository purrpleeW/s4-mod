#pragma once
#include "engine/engine_common.hpp"

namespace t9 {
	enum class netsrc_t {
		NS_NULL = -1,
		NS_CLIENT1 = 0,
		NS_CLIENT2 = 1,
		NS_CLIENT3 = 2,
		NS_CLIENT4 = 3,
		NS_SERVER = 4,
		NS_MAXCLIENTS = 4,
		NS_PACKET = 5
	};
}

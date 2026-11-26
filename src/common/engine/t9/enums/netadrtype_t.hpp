#pragma once
#include "engine/engine_common.hpp"

namespace t9 {
	enum class netadrtype_t {
		NA_BOT = 0,
		NA_BAD = 1,
		NA_LOOPBACK = 2,
		NA_RAWIP = 3,
		NA_IP = 4
	};
}

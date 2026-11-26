#pragma once
#include "engine/engine_common.hpp"

namespace s4 {
	enum class netadrtype_t {
		NA_BAD = 0,
		NA_LOOPBACK = 1,
		NA_BROADCAST = 2,
		NA_IP = 3,
		NA_RAW = 4
	};
}

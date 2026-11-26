#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class bdNATType {
		BD_NAT_UNKNOWN = 0,
		BD_NAT_OPEN = 1,
		BD_NAT_MODERATE = 2,
		BD_NAT_STRICT = 3,
		BD_NAT_COUNT = 4
	};
}

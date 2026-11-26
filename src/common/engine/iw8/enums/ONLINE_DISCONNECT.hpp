#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class ONLINE_DISCONNECT : std::uint8_t {
		ONLINE_DISCONNECT_DEMONWARE = 0,
		ONLINE_DISCONNECT_PLATFORM = 1,
		ONLINE_DISCONNECT_ALL = 2
	};
}

#pragma once
#include "engine/engine_common.hpp"

namespace s4 {
	enum class StatsSource : std::int8_t {
		STATS_ONLINE = 0,
		STATS_OFFLINE = 1,
		STATS_COUNT = 2
	};
}

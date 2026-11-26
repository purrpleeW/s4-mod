#pragma once
#include "engine/engine_common.hpp"

namespace s4 {
	enum ScrThreadMode : std::int8_t {
		MAIN = 0,
		ASYNC_SERVER = 1
	};
}

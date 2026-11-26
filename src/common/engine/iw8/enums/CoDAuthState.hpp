#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class CoDAuthState : std::int32_t {
		AUTHSTATE_NONE = 0,
		AUTHSTATE_UNK1 = 1,
		AUTHSTATE_SUCCESS = 2,
		AUTHSTATE_FAILURE = 3
	};
}

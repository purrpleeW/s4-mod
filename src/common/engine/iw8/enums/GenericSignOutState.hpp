#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class GenericSignOutState {
		SignedOutFromProfile = 0,
		SignedOutFromOnline = 1,
		SignedOutFromLocalClient = 2
	};
}

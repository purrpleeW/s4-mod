#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class _XUSER_SIGNIN_STATE {
		eXUserSigninState_NotSignedIn = 0,
		eXUserSigninState_SignedInLocally = 1,
		eXUserSigninState_SignedInToLive = 2
	};
}

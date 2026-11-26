#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class GenericSignInState {
		NotSignedIn = 0,
		SignedInLocally = 1,
		SignedInToFirstParty = 2,
		SignedInToDemonWare = 3,
		SignedInToLocalClient = 4
	};
}

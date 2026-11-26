#pragma once
#include "engine/engine_common.hpp"

namespace t9 {
	enum class connstate_t {
		CA_DISCONNECTED = 0,
		CA_CINEMATIC = 1,
		CA_UICINEMATIC = 2,
		CA_LOGO = 3,
		CA_CONNECTING = 4,
		CA_UISERVER = 5,
		CA_CONFIRMLOADING = 6,
		CA_CONNECTED = 7,
		CA_SENDINGDATA = 8,
		CA_SENDINGDATA_WAIT = 9,
		CA_LOADING = 10,
		CA_PRIMED = 11,
		CA_ACTIVE = 12,
		CA_COUNT = 13
	};
}

#pragma once
#include "engine/engine_common.hpp"

namespace steam {
	enum class EMatchMakingServerResponse {
		eServerResponded = 0,
		eServerFailedToRespond,
		eNoServersListedOnMasterServer
	};
}

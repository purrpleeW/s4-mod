#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	enum class OnlineChatMessageProfanityFilterState {
		NOT_FILTERED,
		REQUIRES_FILTER,
		FILTERING,
		FILTERED
	};
}

#pragma once
#include "engine/engine_common.hpp"

namespace t9 {
	enum class UIContextIndex_t : std::uint32_t {
		INVALID_UI_CONTEXT = 0xFFFFFFFF,
		UI_CONTEXT_FRONTEND = 0,
		UI_CONTEXT_INDEX_0 = 0,
		UI_CONTEXT_COUNT = 1
	};
}

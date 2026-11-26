#pragma once
#include "engine/engine_common.hpp"

namespace t9 {
	class LocalizeEntry {
	public:
		const char* value_;	// 0x0000
		std::int64_t hash_;	// 0x0008
	};
	ENGINE_ASSERT_SZ(LocalizeEntry, 0x0010);
}

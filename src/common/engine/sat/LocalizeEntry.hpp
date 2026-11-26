#pragma once
#include "engine/engine_common.hpp"

namespace sat {
	class LocalizeEntry {
	public:
		std::uint64_t hash_;	// 0x0000
		const char* value_;		// 0x0008
	};
	ENGINE_ASSERT_SZ(LocalizeEntry, 0x0010);
}

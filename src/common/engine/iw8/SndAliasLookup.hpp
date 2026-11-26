#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class SndAliasLookup {
	public:
		const char* name_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(SndAliasLookup, 0x0008);
}

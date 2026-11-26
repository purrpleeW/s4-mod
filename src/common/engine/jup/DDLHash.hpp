#pragma once
#include "engine/engine_common.hpp"

namespace jup {
	class DDLHash {
	public:
		std::uint32_t hash_;	// 0x0000
		int index_;				// 0x0004
	};
	ENGINE_ASSERT_SZ(DDLHash, 0x0008);
}

#pragma once
#include "engine/engine_common.hpp"

namespace t8 {
	class AssetRef_t {
	public:
		std::int64_t hash_;		// 0x0000
	private:
		char pad_0008[0x0008];	// 0x0008
	};
	ENGINE_ASSERT_SZ(AssetRef_t, 0x0010);
}

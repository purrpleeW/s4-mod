#pragma once
#include "engine/engine_common.hpp"

namespace sat {
	class WeaponCompleteDef {
	public:
		std::uint64_t hash_;			// 0x0000
		const char* sz_internal_name_;	// 0x0008
	private:
		char pad_0010[0x0548];			// 0x0010
	};
	ENGINE_ASSERT_SZ(WeaponCompleteDef, 0x0558);
}

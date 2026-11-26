#pragma once
#include "engine/engine_common.hpp"
#include "engine/t10/DvarLimits.hpp"
#include "engine/t10/DvarValue.hpp"
#include "engine/t10/enums/DvarFlags.hpp"
#include "engine/t10/enums/DvarType.hpp"

namespace t10 {
	class dvar_t {
	public:
		XHash hash_;			// 0x0000
		XHash hash_next_;		// 0x0008
		DvarFlags flags_;		// 0x0010
		std::uint8_t level_[1];	// 0x0014
		DvarType type_;			// 0x0015
		bool modified_;			// 0x0016
	private:
		char pad_0017[0x0001];	// 0x0017
	public:
		DvarLimits domain_;		// 0x0018
		DvarValue current_;		// 0x0028
		DvarValue latched_;		// 0x0038
		DvarValue reset_;		// 0x0048
	};
	ENGINE_ASSERT_SZ(dvar_t, 0x0058);
}

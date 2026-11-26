#pragma once
#include "engine/engine_common.hpp"
#include "engine/jup/DvarLimits.hpp"
#include "engine/jup/DvarValue.hpp"
#include "engine/jup/enums/DvarType.hpp"

namespace jup {
	class dvar_t {
	public:
		std::uint64_t hash_;	// 0x0000
	private:
		char pad_0008[0x000D];	// 0x0008
	public:
		DvarType type_;			// 0x0015
	private:
		char pad_0016[0x0002];	// 0x0016
	public:
		DvarLimits domain_;		// 0x0018
		DvarValue current_;		// 0x0028
	private:
		char pad_0038[0x0010];	// 0x0038
	public:
		DvarValue reset_;		// 0x0048
	};
	ENGINE_ASSERT_SZ(dvar_t, 0x0058);
}

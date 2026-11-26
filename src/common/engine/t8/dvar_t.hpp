#pragma once
#include "engine/engine_common.hpp"
#include "engine/t8/AssetRef_t.hpp"
#include "engine/t8/DvarData.hpp"
#include "engine/t8/DvarLimits.hpp"
#include "engine/t8/enums/dvarType_t.hpp"

namespace t8 {
	class dvar_t {
	public:
		AssetRef_t name_;		// 0x0000
	private:
		char pad_0010[0x0008];	// 0x0010
	public:
		DvarData* value_;		// 0x0018
		dvarType_t type_;		// 0x0020
		std::uint32_t flags_;	// 0x0024
		DvarLimits domain_;		// 0x0028
	private:
		char pad_0038[0x0008];	// 0x0038
	};
	ENGINE_ASSERT_SZ(dvar_t, 0x0040);
}

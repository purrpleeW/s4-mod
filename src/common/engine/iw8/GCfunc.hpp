#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/GCfuncC.hpp"
#include "engine/iw8/GCfuncL.hpp"

namespace iw8 {
	union GCfunc {
		GCfuncC c_;	// 0x0000
		GCfuncL l_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(GCfunc, 0x0038);
}

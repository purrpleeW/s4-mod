#pragma once
#include "engine/engine_common.hpp"
#include "engine/s2/hks/Breakpoint.hpp"

namespace s2::hks {
	class BreakpointList {
	public:
		Breakpoint* list_[128];	// 0x0000
		Debugger* parent_;		// 0x0400
		Breakpoint* first_;		// 0x0408
	};
	ENGINE_ASSERT_SZ(BreakpointList, 0x0410);
}

#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/_jmp_buf.hpp"

namespace t9::hks {
	class CallSite {
	public:
		jmp_buf jump_buffer_;	// 0x0000
		CallSite* prev_;		// 0x0060
	};
	ENGINE_ASSERT_SZ(CallSite, 0x0068);
}

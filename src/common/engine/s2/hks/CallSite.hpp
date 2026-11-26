#pragma once
#include "engine/engine_common.hpp"
#include "engine/s2/_jmp_buf.hpp"

namespace s2::hks {
	class CallSite {
	public:
		jmp_buf jump_buffer_;	// 0x0000
		CallSite* prev_;		// 0x0060
	};
	ENGINE_ASSERT_SZ(CallSite, 0x0068);
}

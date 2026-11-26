#pragma once
#include "engine/engine_common.hpp"

namespace t10 {
	class cmd_function_s {
	public:
		cmd_function_s* next_;					// 0x0000
		const char* name_;						// 0x0008
		void(__fastcall* function_)();			// 0x0010
	};
	static_assert(offsetof(cmd_function_s, function_) == 0x10);
	ENGINE_ASSERT_SZ(cmd_function_s, 0x0018);
}

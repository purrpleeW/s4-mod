#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw9/cmd_function_s.hpp"

namespace iw9 {
	class ClCommandInfo {
	public:
		const char* name_;			// 0x0000
		void (__fastcall* func_)();	// 0x0008
		cmd_function_s var_;		// 0x0010
	};
	ENGINE_ASSERT_SZ(ClCommandInfo, 0x0028);
}

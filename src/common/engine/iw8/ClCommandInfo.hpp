#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/cmd_function_s.hpp"

namespace iw8 {
	class ClCommandInfo {
	public:
		const char* name_;			// 0x0000
		void (__fastcall* func_)();	// 0x0008
		cmd_function_s var_;		// 0x0010

		ClCommandInfo* get_next();
	};
	ENGINE_ASSERT_SZ(ClCommandInfo, 0x0038);
}

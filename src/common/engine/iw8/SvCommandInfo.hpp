#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/cmd_function_s.hpp"

namespace iw8 {
	class SvCommandInfo {
	public:
		const char* name_;			// 0x0000
		void (__fastcall* func_)();	// 0x0008
		cmd_function_s cvvar_;		// 0x0010
		cmd_function_s svvar_;		// 0x0038

		SvCommandInfo* get_next();
	};
	ENGINE_ASSERT_SZ(SvCommandInfo, 0x0060);
}

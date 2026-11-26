#pragma once
#include "engine/engine_common.hpp"
#include "engine/s4/cmd_function_s.hpp"

namespace s4 {
	class SvCommandInfo {
	public:
		const char* name_;
		void (__fastcall* func_)();
		cmd_function_s cvvar_;
		cmd_function_s svvar_;

		SvCommandInfo* get_next();   // ← add this
	};
	ENGINE_ASSERT_SZ(SvCommandInfo, 0x0040);
}
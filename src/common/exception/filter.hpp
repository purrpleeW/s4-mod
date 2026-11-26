#pragma once
#include "common_core.hpp"
#include "nlohmann/json.hpp"

namespace exception {
	void ensure_dbg_help();
	nlohmann::json generate_exception_json(EXCEPTION_POINTERS* inf);
	bool launch_external_crash_handler(const nlohmann::json& arg);
	std::vector<std::uintptr_t> get_callstack(const CONTEXT* ctx);
	long WINAPI filter(EXCEPTION_POINTERS* inf);
}

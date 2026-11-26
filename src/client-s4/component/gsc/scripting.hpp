#pragma once
#include "common.hpp"

#include <utils/concurrency.hpp>

namespace gsc::scripting {
	extern std::unordered_map<int, std::unordered_map<std::string, int>> fields_table;
	extern std::unordered_map<std::string, std::unordered_map<std::string, const char*>> script_function_table;
	extern std::unordered_map<std::string, std::vector<std::pair<std::string, const char*>>> script_function_table_sort;
	extern std::unordered_map<const char*, std::pair<std::string, std::string>> script_function_table_rev;

	extern std::string current_file;

	bool should_enable_injector();
	void on_shutdown(const std::function<void(bool, bool)>& callback);
	std::string get_token(std::uint32_t id);
}

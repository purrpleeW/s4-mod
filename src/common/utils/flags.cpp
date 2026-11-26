#include "flags.hpp"
#include "common_core.hpp"
#include "string.hpp"
#include "nt.hpp"

#include <shellapi.h>
#include <stdlib.h>
#include <unordered_set>
#include <ranges>

#include "utils/string.hpp"

namespace utils::flags {
	static auto& parsed_flags = *new std::unordered_map<std::string, std::string>();

	void parse_flags(const int argc, char** argv) {
		parsed_flags.reserve(argc);
		for (int i = 0; i < argc; ++i) {
			const auto arg = argv[i];

			if (*arg != '-') {
				continue;
			}

			bool is_next_arg_value = (i + 1 < argc && *argv[i + 1] != '-');

			const auto cleaned_arg = string::to_lower(std::string(arg + 1 /*skips the -*/));
			parsed_flags[cleaned_arg] = is_next_arg_value ? std::string(argv[i + 1]) : "";
		}
	}

	void parse_flags() {
		int args_c = 0;
		auto* args_v = CommandLineToArgvW(GetCommandLineW(), &args_c);
		if (!args_v) {
			return;
		}

		const std::unique_ptr<void, decltype(&LocalFree)> args_guard(args_v, LocalFree);

		std::vector<std::unique_ptr<char[]>> buffers(args_c);
		std::vector<char*> argv(args_c);

		for (int i = 0; i < args_c; ++i) {
			int bytes_needed = WideCharToMultiByte(
				CP_UTF8, 0, args_v[i], -1, nullptr, 0, nullptr, nullptr
			);
			buffers[i] = std::make_unique<char[]>(bytes_needed);
			WideCharToMultiByte(CP_UTF8, 0, args_v[i], -1, buffers[i].get(), bytes_needed, nullptr, nullptr);
			argv[i] = buffers[i].get();
		}

		return parse_flags(argv.size(), argv.data());
	}

	bool has_flag(const std::string& flag) {
		return parsed_flags.find(string::to_lower(flag)) != parsed_flags.end();
	}

	const std::string get_flag(const std::string& flag) {
		const auto flag_ = parsed_flags.find(string::to_lower(flag));
		if (flag_ != parsed_flags.end()) {
			return flag_->second;
		}

		return "";
	};
}

#pragma once
#include <charconv>
#include <string>

namespace utils::flags {
	void parse_flags(const int argc, char** argv);
	void parse_flags();

	bool has_flag(const std::string& flag);
	const std::string get_flag(const std::string& flag);

	template <typename T>
	inline T get_flag(const std::string& flag) {
		T result = T();
		const auto value = get_flag(flag);
		const auto [_, ec] = std::from_chars(value.data(), value.data() + value.size(), result);
		return (ec == std::errc()) ? result : T();
	}
}

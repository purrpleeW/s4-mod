#pragma once
#include "common_core.hpp"

namespace lua_decompiler {
	constexpr std::uint64_t DOUBLE_SIGN = 0x8000000000000000;
	constexpr std::uint64_t DOUBLE_EXPONENT = 0x7FF0000000000000;
	constexpr std::uint64_t DOUBLE_FRACTION = 0x000FFFFFFFFFFFFF;
	constexpr std::uint64_t DOUBLE_SPECIAL = DOUBLE_EXPONENT;
	constexpr std::uint64_t DOUBLE_NEGATIVE_ZERO = DOUBLE_SIGN;

	struct ld_error {
		const std::string message_;
		const std::string file_path_;
		const std::string function_;
		const std::string source_;
		const std::string line_;
	};

	inline void ld_assert_internal(bool condition, const std::string& message, const std::string& file_path, const std::string& function, const std::string& source, const int& line) {
		if (condition) {
			return;
		}
		throw ld_error{
			.message_ = message,
			.file_path_ = file_path,
			.function_ = function,
			.source_ = source,
			.line_ = std::to_string(line)
		};
	}

#	define ld_assert(c, m, f) \
		ld_assert_internal(c, m, f, __FUNCTION__, __FILE__, __LINE__)
}

#pragma once
#include "utils/flags.hpp"

namespace ci {

	enum TestType {
		Undefined = 0,
		Signature,
		Hook,
	};

	enum TestState {
		Started,
		InProgress,
		Ended,
		Failure,
	};

	inline bool is_testing() {
		#ifndef CLIENT_PRERELEASE
			return false;
		#else
			return utils::flags::has_flag("ci");
		#endif
	}

	void begin_test_run();
	void record_test_state(const TestState state, const TestType type,
		const std::string_view test_name, std::string test_data);
	void end_test_run(const int exit_code);
}
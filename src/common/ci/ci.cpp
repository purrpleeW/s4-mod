#include "ci.hpp"
#include "identification/game.hpp"
#include "identification/utility.hpp"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include <chrono>
#include <deque>
#include <loader/component_loader.hpp>

namespace ci {

	namespace {
		struct TestRecord {
			const std::chrono::system_clock::time_point time_;
			const TestState state_;
			const TestType type_;
			const std::string_view test_name_;
			std::string test_data_;
		};

		std::vector<TestRecord> ci_records{};

		std::string format_time(const std::chrono::system_clock::time_point& tp) {
			std::time_t t = std::chrono::system_clock::to_time_t(tp);
			std::tm tm{};
			localtime_s(&tm, &t);
			std::ostringstream oss;
			oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
			return oss.str();
		}

		const char* to_string(TestState s) {
			switch (s) {
			case TestState::Started:  	  return "Started";
			case TestState::InProgress:   return "In Progress";
			case TestState::Ended:   	  return "Ended";
			case TestState::Failure:      return "Failure";
			default:                	  return "unknown";
			}
		}

		const char* to_string(TestType t) {
			switch (t) {
			case TestType::Undefined:   return "Undefined";
			case TestType::Signature:   return "Signature";
			case TestType::Hook:		return "Hook";
			default:                  	return "unknown";
			}
		}
	}

	void begin_test_run() {
		if (!is_testing()) {
			return;
		}

		LOG("CI", INFO, "Testing was enabled, game will quit upon a Com_Error now!");
		ci_records.reserve(4096); // this should hopefully make it not that shit? i mean its 4096 log lines, cant be that much to test
	}

	void record_test_state(const TestState state, const TestType type,
		const std::string_view test_name, std::string test_data)
	{
		if (!is_testing()) {
			return;
		}

		ci_records.emplace_back(TestRecord{
			.time_ = std::chrono::system_clock::now(),
			.state_ = state,
			.type_ = type,
			.test_name_ = test_name,
			.test_data_ = test_data
		});
	}

	void end_test_run(const int exit_code) {
		if (!is_testing()) {
			return;
		}

		std::ofstream log(std::format("{}-ci_results.log", identification::utility::get_game_deltaless_version()), std::ios::out | std::ios::trunc);

		for (const auto& r : ci_records) {
			log
				<< "[" << format_time(r.time_) << "] "
				<< to_string(r.type_) << " | "
				<< to_string(r.state_) << " | "
				<< r.test_name_ << " | "
				<< r.test_data_
				<< "\n";
		}

		log << "\nExit code: " << exit_code << "\n";
		log.flush();
		log.close();

		exit(exit_code);
	}

}
#pragma once

#include <atomic>
#include <charconv>
#include <optional>
#include <string_view>

namespace online::ui::cod_points {
	inline constexpr int currency_id = 20;
	inline constexpr int native_display = -1;
	inline constexpr int default_balance = 67;
	inline constexpr const char* model_path = "frontEnd.commerce.currency_20";

	inline std::optional<int> parse_display_balance(std::string_view text) {
		if (text == "native") {
			return native_display;
		}
		if (text.empty()) {
			return std::nullopt;
		}

		int value = 0;
		const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
		if (error != std::errc() || end != text.data() + text.size() || value < 0) {
			return std::nullopt;
		}
		return value;
	}

	inline int resolve_display_balance(int setting, int currency, int native_balance) {
		return setting != native_display && currency == currency_id ? setting : native_balance;
	}

	class display_balance {
		std::atomic<int> value_{ default_balance };
	public:
		int get() const {
			return value_.load();
		}

		bool set(int value) {
			if (value < native_display) {
				return false;
			}
			value_ = value;
			return true;
		}
	};
}
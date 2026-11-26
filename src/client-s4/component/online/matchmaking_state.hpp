#pragma once

#include <engine/s4/enums/MatchmakerUIState.hpp>

#include <atomic>
#include <charconv>
#include <cstdint>
#include <optional>
#include <string_view>

namespace online::matchmaking {
	struct ui_state {
		s4::MatchmakerUIState state;
		int extra_info;
	};

	inline std::optional<int> parse_integer(std::string_view text) {
		if (text.empty()) {
			return std::nullopt;
		}
		int value = 0;
		const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
		if (error != std::errc() || end != text.data() + text.size()) {
			return std::nullopt;
		}
		return value;
	}

	class ui_state_override {
		std::atomic<std::uint64_t> value_{};
		static constexpr std::uint64_t enabled_bit = std::uint64_t{ 1 } << 63;
	public:
		bool set(s4::MatchmakerUIState state, int extra_info) {
			if (state < s4::MatchmakerUIState::IDLE || state > s4::MatchmakerUIState::ERRORING || extra_info < 0) {
				return false;
			}
			if (state != s4::MatchmakerUIState::SEARCHING && state != s4::MatchmakerUIState::QOSING) {
				extra_info = 0;
			}
			value_ = enabled_bit | (static_cast<std::uint64_t>(extra_info) << 32) | static_cast<unsigned int>(state);
			return true;
		}

		std::optional<ui_state> get() const {
			const auto value = value_.load();
			if (!(value & enabled_bit)) {
				return std::nullopt;
			}
			return ui_state{ static_cast<s4::MatchmakerUIState>(value & 0xFFFFFFFF),
				static_cast<int>((value & ~enabled_bit) >> 32) };
		}

		void reset() {
			value_ = 0;
		}
	};
}
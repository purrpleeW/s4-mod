#pragma once
#include "common_core.hpp"
#include "logger/ansi.hpp"

namespace logger {
	class log_level {
		const std::string_view label_{};
		const std::string_view ansi_color_{};
	public:
		constexpr log_level(const std::string_view label, const std::string_view ansi_color) noexcept :
			label_(label), ansi_color_(ansi_color)
		{}

		constexpr std::string_view get_label() const noexcept {
			return this->label_;
		}

		constexpr std::string_view get_ansi_color() const noexcept {
			return this->ansi_color_;
		}
	};

	namespace levels {
#		ifdef DEBUG
#			undef DEBUG
#		endif
#		ifdef ERROR
#			undef ERROR
#		endif

		namespace id {
			enum log_levels {
				INFO, WARN, ERROR, FATAL, TRACE, DEBUG
			};
		}

		constexpr log_level log_level_lookup[] = {
			[id::INFO]  = { "INFO",  ANSI_FG_RGB(92, 150, 44)  },
			[id::WARN]  = { "WARN",  ANSI_FG_RGB(166, 138, 13) },
			[id::ERROR] = { "ERROR", ANSI_FG_RGB(240, 82, 79)  },
			[id::FATAL] = { "FATAL", ANSI_FG_STYLE_256(124)    },
			[id::TRACE] = { "TRACE", ANSI_FG_STYLE_256(214)    },
			[id::DEBUG] = { "DEBUG", ANSI_FG_STYLE_256(243)    },
		};

		inline constexpr auto& INFO  = log_level_lookup[id::INFO];
		inline constexpr auto& WARN  = log_level_lookup[id::WARN];
		inline constexpr auto& ERROR = log_level_lookup[id::ERROR];
		inline constexpr auto& FATAL = log_level_lookup[id::FATAL];
		inline constexpr auto& TRACE = log_level_lookup[id::TRACE];
		inline constexpr auto& DEBUG = log_level_lookup[id::DEBUG];
	}
}
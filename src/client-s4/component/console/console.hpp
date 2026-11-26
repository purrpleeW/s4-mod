#pragma once
#include "game/function_types.hpp"

#include <component/console/console_base.hpp>

namespace console {
	namespace detail {
		using game_console = detail::console_provider<s4::GfxFont, s4::vec4_t, true>;
	}

	inline detail::game_console game_console{};
}
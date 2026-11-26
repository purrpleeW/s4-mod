#pragma once
#include "common.hpp"
#include "game/function_types.hpp"

namespace key_catchers {
	using ke_typedef = std::function<bool(int, s4::keyNum_t, bool)>;
	using ch_typedef = std::function<bool(int, s4::keyNum_t)>;

	void add_key_event(ke_typedef&& func);
	void add_char_event(ch_typedef&& func);

	bool get_state();
	void toggle_state();
	void remove_catcher(int and_mask);
}

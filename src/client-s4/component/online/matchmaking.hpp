#pragma once

#include <engine/s4/enums/MatchmakerUIState.hpp>

namespace online::matchmaking {
	bool supports_match_start();
	bool set_state(s4::MatchmakerUIState state, int extra_info = 0);
	void reset();
}
#pragma once

namespace s4 {
	struct MatchmakerUIStateData {
		union {
			int num_qos_players_;
			int max_ping_;
		};
	};
	static_assert(sizeof(MatchmakerUIStateData) == 4);
}
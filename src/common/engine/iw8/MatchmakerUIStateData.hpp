#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class MatchmakerUIStateData {
	public:
		union {
			int num_qos_players_;	// 0x0000
			int max_ping_;			// 0x0000
		};
	};
	ENGINE_ASSERT_SZ(MatchmakerUIStateData, 0x0004);
}

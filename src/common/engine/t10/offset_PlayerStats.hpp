#pragma once
#include "engine/engine_common.hpp"

namespace t10 {
	class offset_PlayerStats {
	public:
		bool stats_fetched_;				// 0x0000
		bool online_stat_write_pending_;	// 0x0001
		bool allow_saving_progress_;		// 0x0002
	private:
		char pad_0003[0x0001];				// 0x0003
	};
	ENGINE_ASSERT_SZ(offset_PlayerStats, 0x0004);
}

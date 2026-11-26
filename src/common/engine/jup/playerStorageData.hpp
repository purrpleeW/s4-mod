#pragma once
#include "engine/engine_common.hpp"
#include "engine/jup/enums/StatsSource.hpp"

namespace jup {
	class playerStorageData {
	public:
		bool use_game_stats_source_;		// 0x0000
	private:
		char pad_0001[0x0003];				// 0x0001
	public:
		StatsSource game_stats_source_;		// 0x0004
		bool resetting_stats_;				// 0x0008
	private:
		char pad_0009[0x0003];				// 0x0009
	public:
		StatsSource reset_stats_source_;	// 0x000C
	};
	ENGINE_ASSERT_SZ(playerStorageData, 0x0010);
}

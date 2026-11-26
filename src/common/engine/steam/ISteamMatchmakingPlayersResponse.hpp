#pragma once
#include "engine/engine_common.hpp"

namespace steam {
	class ISteamMatchmakingPlayersResponse {
	public:
		virtual void AddPlayerToList(const char* name, int score, float time_played) = 0;
		virtual void PlayersFailedToRespond() = 0;
		virtual void PlayersRefreshComplete() = 0;
	};
	ENGINE_ASSERT_SZ(ISteamMatchmakingPlayersResponse, 0x0008);
}

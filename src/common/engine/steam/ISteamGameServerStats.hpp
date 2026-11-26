#pragma once
#include "engine/engine_common.hpp"
#include "engine/steam/CSteamID.hpp"

namespace steam {
	class ISteamGameServerStats {
	public:
		virtual SteamAPICall_t RequestUserStats(CSteamID user) = 0;
		virtual bool GetUserStat(CSteamID user, const char* name, int* data) = 0;
		virtual bool GetUserStat(CSteamID user, const char* name, float* data) = 0;
		virtual bool GetUserAchievement(CSteamID user, const char* name, bool* achieved) = 0;
		virtual bool SetUserStat(CSteamID user, const char* name, int data) = 0;
		virtual bool SetUserStat(CSteamID user, const char* name, float data) = 0;
		virtual bool UpdateUserAvgRateStat(CSteamID user, const char* name, float count_this_session, double session_length) = 0;
		virtual bool SetUserAchievement(CSteamID user, const char* name) = 0;
		virtual bool ClearUserAchievement(CSteamID user, const char* name) = 0;
		virtual SteamAPICall_t StoreUserStats(CSteamID user) = 0;
	};
	ENGINE_ASSERT_SZ(ISteamGameServerStats, 0x0008);
}

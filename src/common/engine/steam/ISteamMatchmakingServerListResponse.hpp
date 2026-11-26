#pragma once
#include "engine/engine_common.hpp"
#include "engine/steam/enums/EMatchMakingServerResponse.hpp"

namespace steam {
	class ISteamMatchmakingServerListResponse {
	public:
		virtual void ServerResponded(HServerListRequest request, int server) = 0;
		virtual void ServerFailedToRespond(HServerListRequest request, int server) = 0;
		virtual void RefreshComplete(HServerListRequest request, EMatchMakingServerResponse response) = 0;
	};
	ENGINE_ASSERT_SZ(ISteamMatchmakingServerListResponse, 0x0008);
}

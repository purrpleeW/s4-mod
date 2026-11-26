#pragma once
#include "engine/engine_common.hpp"
#include "engine/steam/SteamIPAddress_t.hpp"
#include "engine/steam/enums/EAccountType.hpp"

namespace steam {
	class ISteamClient {
	public:
		virtual HSteamPipe CreateSteamPipe() = 0;
		virtual bool BReleaseSteamPipe(HSteamPipe pipe) = 0;
		virtual HSteamUser ConnectToGlobalUser(HSteamPipe pipe) = 0;
		virtual HSteamUser CreateLocalUser(HSteamPipe* pipe, EAccountType account_type) = 0;
		virtual void ReleaseUser(HSteamPipe pipe, HSteamUser user) = 0;
		virtual ISteamUser* GetISteamUser(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual ISteamGameServer* GetISteamGameServer(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual void SetLocalIPBinding(const SteamIPAddress_t& ip, std::uint16_t port) = 0;
		virtual ISteamFriends* GetISteamFriends(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual ISteamUtils* GetISteamUtils(HSteamPipe pipe, const char* version) = 0;
		virtual ISteamMatchmaking* GetISteamMatchmaking(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual ISteamMatchmakingServers* GetISteamMatchmakingServers(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual void* GetISteamGenericInterface(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual ISteamUserStats* GetISteamUserStats(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual ISteamGameServerStats* GetISteamGameServerStats(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual ISteamApps* GetISteamApps(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual ISteamNetworking* GetISteamNetworking(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual ISteamRemoteStorage* GetISteamRemoteStorage(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual ISteamScreenshots* GetISteamScreenshots(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual ISteamGameSearch* GetISteamGameSearch(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual void RunFrame() = 0;
		virtual std::uint32_t GetIPCCallCount() = 0;
		virtual void SetWarningMessageHook(SteamAPIWarningMessageHook_t func) = 0;
		virtual bool BShutdownIfAllPipesClosed() = 0;
		virtual ISteamHTTP* GetISteamHTTP(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual ISteamUnifiedMessages* GetISteamUnifiedMessages(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual ISteamController* GetISteamController(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual ISteamUGC* GetISteamUGC(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual ISteamAppList* GetISteamAppList(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual ISteamMusic* GetISteamMusic(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual ISteamMusicRemote* GetISteamMusicRemote(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual ISteamHTMLSurface* GetISteamHTMLSurface(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual void Set_SteamAPI_CPostAPIResultInProcess(void(*)()) = 0;
		virtual void Remove_SteamAPI_CPostAPIResultInProcess(void(*)()) = 0;
		virtual void Set_SteamAPI_CCheckCallbackRegisteredInProcess(SteamAPI_CheckCallbackRegistered_t func) = 0;
		virtual ISteamInventory* GetISteamInventory(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual ISteamVideo* GetISteamVideo(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual ISteamParentalSettings* GetISteamParentalSettings(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual ISteamInput* GetISteamInput(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual ISteamParties* GetISteamParties(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual ISteamRemotePlay* GetISteamRemotePlay(HSteamUser user, HSteamPipe pipe, const char* version) = 0;
		virtual void DestroyAllInterfaces() = 0;
	};
}

#pragma once
#include "engine/engine_common.hpp"
#include "engine/steam/CSteamID.hpp"

namespace steam {
	class ISteamApps {
	public:
		virtual bool BIsSubscribed() = 0;
		virtual bool BIsLowViolence() = 0;
		virtual bool BIsCybercafe() = 0;
		virtual bool BIsVACBanned() = 0;
		virtual const char* GetCurrentGameLanguage() = 0;
		virtual const char* GetAvailableGameLanguages() = 0;
		virtual bool BIsSubscribedApp(AppId_t app_id) = 0;
		virtual bool BIsDlcInstalled(AppId_t app_id) = 0;
		virtual std::uint32_t GetEarliestPurchaseUnixTime(AppId_t app_id) = 0;
		virtual bool BIsSubscribedFromFreeWeekend() = 0;
		virtual int GetDLCCount() = 0;
		virtual bool BGetDLCDataByIndex(int dlc, AppId_t* app_id, bool* available, char* name, int name_buffer_size) = 0;
		virtual void InstallDLC(AppId_t app_id) = 0;
		virtual void UninstallDLC(AppId_t app_id) = 0;
		virtual void RequestAppProofOfPurchaseKey(AppId_t app_id) = 0;
		virtual bool GetCurrentBetaName(char* name, int name_buffer_size) = 0;
		virtual bool MarkContentCorrupt(bool missing_files_only) = 0;
		virtual std::uint32_t GetInstalledDepots(AppId_t app_id, DepotId_t* depots, std::uint32_t max_depots) = 0;
		virtual std::uint32_t GetAppInstallDir(AppId_t app_id, char* folder, std::uint32_t folder_buffer_size) = 0;
		virtual bool BIsAppInstalled(AppId_t app_id) = 0;
		virtual CSteamID GetAppOwner() = 0;
		virtual const char* GetLaunchQueryParam(const char* key) = 0; 
		virtual bool GetDlcDownloadProgress(AppId_t app_id, std::uint64_t* bytes_downloaded, std::uint64_t* bytes_total) = 0;
		virtual int GetAppBuildId() = 0;
		virtual void RequestAllProofOfPurchaseKeys() = 0;
		virtual SteamAPICall_t GetFileDetails(const char* file_name) = 0;
		virtual int GetLaunchCommandLine(char* buffer, int buffer_size) = 0;
		virtual bool BIsSubscribedFromFamilySharing() = 0;
		virtual bool BIsTimedTrial(std::uint32_t* seconds_allowed, std::uint32_t* seconds_played) = 0;
	};
	ENGINE_ASSERT_SZ(ISteamApps, 0x0008);
}

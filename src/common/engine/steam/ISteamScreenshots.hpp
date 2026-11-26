#pragma once
#include "engine/engine_common.hpp"
#include "engine/steam/CSteamID.hpp"
#include "engine/steam/enums/EVRScreenshotType.hpp"

namespace steam {
	class ISteamScreenshots {
	public:
		virtual ScreenshotHandle WriteScreenshot(void* rgb_buffer, std::uint32_t rgb_buffer_size, int width, int height) = 0;
		virtual ScreenshotHandle AddScreenshotToLibrary(const char* filename, const char* thumbnail_filename, int width, int height) = 0;
		virtual void TriggerScreenshot() = 0;
		virtual void HookScreenshots(bool hook) = 0;
		virtual bool SetLocation(ScreenshotHandle screenshot, const char* location) = 0;
		virtual bool TagUser(ScreenshotHandle screenshot, CSteamID steam_id) = 0;
		virtual bool TagPublishedFile(ScreenshotHandle screenshot, PublishedFileId_t published_file_id) = 0;
		virtual bool IsScreenshotsHooked() = 0;
		virtual ScreenshotHandle AddVRScreenshotToLibrary(EVRScreenshotType type, const char* filename, const char* vr_filename) = 0;
	};
	ENGINE_ASSERT_SZ(ISteamScreenshots, 0x0008);
}

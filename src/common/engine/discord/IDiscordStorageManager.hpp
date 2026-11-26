#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/DiscordFileStat.hpp"
#include "engine/discord/enums/EDiscordResult.hpp"

namespace discord {
	class IDiscordStorageManager {
	public:
		EDiscordResult (*read)(IDiscordStorageManager* manager, const char* name, std::uint8_t* data,						// 0x0000
			std::uint32_t data_length, std::uint32_t* read);
		void (*read_async)(IDiscordStorageManager* manager, const char* name, void* callback_data,							// 0x0008
			void (*callback)(void* callback_data, EDiscordResult result, std::uint8_t* data, std::uint32_t data_length));
		void (*read_async_partial)(IDiscordStorageManager* manager, const char* name, std::uint64_t offset,					// 0x0010
			std::uint64_t length, void* callback_data,
			void (*callback)(void* callback_data, EDiscordResult result, std::uint8_t* data, std::uint32_t data_length));
		EDiscordResult (*write)(IDiscordStorageManager* manager, const char* name, std::uint8_t* data,						// 0x0018
			std::uint32_t data_length);
		void (*write_async)(IDiscordStorageManager* manager, const char* name, std::uint8_t* data,							// 0x0020
			std::uint32_t data_length, void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
		EDiscordResult (*delete_)(IDiscordStorageManager* manager, const char* name);										// 0x0028
		EDiscordResult (*exists)(IDiscordStorageManager* manager, const char* name, bool* exists);							// 0x0030
		void (*count)(IDiscordStorageManager* manager, int* count);															// 0x0038
		EDiscordResult (*stat)(IDiscordStorageManager* manager, const char* name, DiscordFileStat* stat);					// 0x0040
		EDiscordResult (*stat_at)(IDiscordStorageManager* manager, int index, DiscordFileStat* stat);						// 0x0048
		EDiscordResult (*get_path)(IDiscordStorageManager* manager, DiscordPath* path);										// 0x0050
	};
	ENGINE_ASSERT_SZ(IDiscordStorageManager, 0x0058);
}

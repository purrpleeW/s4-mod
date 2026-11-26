#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/DiscordActivityAssets.hpp"
#include "engine/discord/DiscordActivityParty.hpp"
#include "engine/discord/DiscordActivitySecrets.hpp"
#include "engine/discord/DiscordActivityTimestamps.hpp"
#include "engine/discord/enums/EDiscordActivityType.hpp"

namespace discord {
#	pragma pack(push, 4)
	class DiscordActivity {
	public:
		EDiscordActivityType type_;				// 0x0000
	private:
		char pad_0004[0x0004];					// 0x0004
	public:
		std::int64_t application_id_;			// 0x0008
		char name_[128];						// 0x0010
		char state_[128];						// 0x0090
		char details_[128];						// 0x0110
		DiscordActivityTimestamps timestamps_;	// 0x0190
		DiscordActivityAssets assets_;			// 0x01A0
		DiscordActivityParty party_;			// 0x03A0
		DiscordActivitySecrets secrets_;		// 0x042C
		bool instance_;							// 0x05AC
	private:
		char pad_05AD[0x0003];					// 0x05AD
	public:
		std::uint32_t supported_platforms_;		// 0x05B0
	};
#	pragma pack(pop)
	ENGINE_ASSERT_SZ(DiscordActivity, 0x05B4);
}

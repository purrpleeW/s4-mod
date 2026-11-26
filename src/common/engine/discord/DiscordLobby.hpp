#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/enums/EDiscordLobbyType.hpp"

namespace discord {
	class DiscordLobby {
	public:
		DiscordLobbyId id_;			// 0x0000
		EDiscordLobbyType type_;	// 0x0008
	private:
		char pad_000C[0x0004];		// 0x000C
	public:
		DiscordUserId owner_id_;	// 0x0010
		DiscordLobbySecret secret_;	// 0x0018
		std::uint32_t capacity_;	// 0x0098
		bool locked_;				// 0x009C
	private:
		char pad_009D[0x0003];		// 0x009D
	};
	ENGINE_ASSERT_SZ(DiscordLobby, 0x00A0);
}

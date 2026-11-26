#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/enums/EDiscordLobbySearchCast.hpp"
#include "engine/discord/enums/EDiscordLobbySearchComparison.hpp"
#include "engine/discord/enums/EDiscordLobbySearchDistance.hpp"
#include "engine/discord/enums/EDiscordResult.hpp"

namespace discord {
	class IDiscordLobbySearchQuery {
	public:
		EDiscordResult (*filter)(IDiscordLobbySearchQuery* lobby_search_query, DiscordMetadataKey key, EDiscordLobbySearchComparison comparison,	// 0x0000
			EDiscordLobbySearchCast cast, DiscordMetadataValue value);
		EDiscordResult (*sort)(IDiscordLobbySearchQuery* lobby_search_query, DiscordMetadataKey key, EDiscordLobbySearchCast cast,					// 0x0008
			DiscordMetadataValue value);
		EDiscordResult (*limit)(IDiscordLobbySearchQuery* lobby_search_query, std::uint32_t limit);													// 0x0010
		EDiscordResult (*distance)(IDiscordLobbySearchQuery* lobby_search_query, EDiscordLobbySearchDistance distance);								// 0x0018
	};
	ENGINE_ASSERT_SZ(IDiscordLobbySearchQuery, 0x0020);
}

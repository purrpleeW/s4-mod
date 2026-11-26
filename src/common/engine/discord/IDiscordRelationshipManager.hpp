#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/DiscordRelationship.hpp"
#include "engine/discord/enums/EDiscordResult.hpp"

namespace discord {
	class IDiscordRelationshipManager {
	public:
		void (*filter)(IDiscordRelationshipManager* manager, void* filter_data,													// 0x0000
			bool (*filter)(void* filter_data, DiscordRelationship* relationship));
		EDiscordResult (*count)(IDiscordRelationshipManager* manager, int* count);												// 0x0008
		EDiscordResult (*get)(IDiscordRelationshipManager* manager, DiscordUserId user_id, DiscordRelationship* relationship);	// 0x0010
		EDiscordResult (*get_at)(IDiscordRelationshipManager* manager, std::uint32_t index, DiscordRelationship* relationship);	// 0x0018
	};
	ENGINE_ASSERT_SZ(IDiscordRelationshipManager, 0x0020);
}

#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/DiscordRelationship.hpp"

namespace discord {
	class IDiscordRelationshipEvents {
	public:
		void (*on_refresh)(void* event_data);													// 0x0000
		void (*on_relationship_update)(void* event_data, DiscordRelationship* relationship);	// 0x0008
	};
	ENGINE_ASSERT_SZ(IDiscordRelationshipEvents, 0x0010);
}

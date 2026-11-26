#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/DiscordEntitlement.hpp"

namespace discord {
	class IDiscordStoreEvents {
	public:
		void (*on_entitlement_create)(void* event_data, DiscordEntitlement* entitlement);
		void (*on_entitlement_delete)(void* event_data, DiscordEntitlement* entitlement);
	};
}

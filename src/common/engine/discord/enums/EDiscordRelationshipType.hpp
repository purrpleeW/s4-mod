#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	enum class EDiscordRelationshipType {
		DiscordRelationshipType_None,
		DiscordRelationshipType_Friend,
		DiscordRelationshipType_Blocked,
		DiscordRelationshipType_PendingIncoming,
		DiscordRelationshipType_PendingOutgoing,
		DiscordRelationshipType_Implicit
	};
}

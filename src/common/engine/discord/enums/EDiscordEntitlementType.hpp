#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	enum class EDiscordEntitlementType {
		DiscordEntitlementType_Purchase = 1,
		DiscordEntitlementType_PremiumSubscription,
		DiscordEntitlementType_DeveloperGift,
		DiscordEntitlementType_TestModePurchase,
		DiscordEntitlementType_FreePurchase,
		DiscordEntitlementType_UserGift,
		DiscordEntitlementType_PremiumPurchase
	};
}

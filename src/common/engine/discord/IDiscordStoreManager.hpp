#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/DiscordEntitlement.hpp"
#include "engine/discord/DiscordSku.hpp"
#include "engine/discord/enums/EDiscordResult.hpp"

namespace discord {
	class IDiscordStoreManager {
	public:
		void (*fetch_skus)(IDiscordStoreManager* manager, void* callback_data,												// 0x0000
			void (*callback)(void* callback_data, EDiscordResult result));
		void (*count_skus)(IDiscordStoreManager* manager, int* count);														// 0x0008
		EDiscordResult (*get_sku)(IDiscordStoreManager* manager, DiscordSnowflake sku_id, DiscordSku* sku);					// 0x0010
		EDiscordResult (*get_sku_at)(IDiscordStoreManager* manager, int index, DiscordSku* sku);							// 0x0018
		void (*fetch_entitlements)(IDiscordStoreManager* manager, void* callback_data,										// 0x0020
			void (*callback)(void* callback_data, EDiscordResult result));
		void (*count_entitlements)(IDiscordStoreManager* manager, int* count);												// 0x0028
		EDiscordResult (*get_entitlement)(IDiscordStoreManager* manager, DiscordSnowflake entitlement_id,					// 0x0030
			DiscordEntitlement* entitlement);
		EDiscordResult (*get_entitlement_at)(IDiscordStoreManager* manager, int index, DiscordEntitlement* entitlement);	// 0x0038
		EDiscordResult (*has_sku_entitlement)(IDiscordStoreManager* manager, DiscordSnowflake sku_id,						// 0x0040
			bool* has_entitlement);
		void (*start_purchase)(IDiscordStoreManager* manager, DiscordSnowflake sku_id, void* callback_data,					// 0x0048
			void (*callback)(void* callback_data, EDiscordResult result));
	};
	ENGINE_ASSERT_SZ(IDiscordStoreManager, 0x0050);
}

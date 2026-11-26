#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/DiscordImeUnderline.hpp"
#include "engine/discord/DiscordRect.hpp"
#include "engine/discord/enums/EDiscordActivityActionType.hpp"
#include "engine/discord/enums/EDiscordMouseButton.hpp"
#include "engine/discord/enums/EDiscordKeyVariant.hpp"
#include "engine/discord/enums/EDiscordResult.hpp"

namespace discord {
	class IDiscordOverlayManager {
	public:
		void (*is_enabled)(IDiscordOverlayManager* manager, bool* enabled);														// 0x0000
		void (*is_locked)(IDiscordOverlayManager* manager, bool* locked);														// 0x0008
		void (*set_locked)(IDiscordOverlayManager* manager, bool locked, void* callback_data,									// 0x0010
			void (*callback)(void* callback_data, EDiscordResult result));
		void (*open_activity_invite)(IDiscordOverlayManager* manager, EDiscordActivityActionType type,							// 0x0018
			void* callback_data, void (*callback)(void* callback_data, EDiscordResult result));
		void (*open_guild_invite)(IDiscordOverlayManager* manager, const char* code, void* callback_data,						// 0x0020
			void (*callback)(void* callback_data, EDiscordResult result));
		void (*open_voice_settings)(IDiscordOverlayManager* manager, void* callback_data,										// 0x0028
			void (*callback)(void* callback_data, EDiscordResult result));
		EDiscordResult (*init_drawing_dxgi)(IDiscordOverlayManager* manager, IDXGISwapChain* swapchain,							// 0x0030
			bool use_message_forwarding);
		void (*on_present)(IDiscordOverlayManager* manager);																	// 0x0038
		void (*forward_message)(IDiscordOverlayManager* manager, MSG* message);													// 0x0040
		void (*key_event)(IDiscordOverlayManager* manager, bool down, const char* key_code,										// 0x0048
			EDiscordKeyVariant variant);
		void (*char_event)(IDiscordOverlayManager* manager, const char* character);												// 0x0050
		void (*mouse_button_event)(IDiscordOverlayManager* manager, std::uint8_t down, int click_count,							// 0x0058
			EDiscordMouseButton which, int x, int y);
		void (*mouse_motion_event)(IDiscordOverlayManager* manager, int x, int y);												// 0x0060
		void (*ime_commit_text)(IDiscordOverlayManager* manager, const char* text);												// 0x0068
		void (*ime_set_composition)(IDiscordOverlayManager* manager, const char* text,											// 0x0070
			DiscordImeUnderline* underlines, std::uint32_t underlines_length, int from, int to);
		void (*ime_cancel_composition)(IDiscordOverlayManager* manager);														// 0x0078
		void (*set_ime_composition_range_callback)(IDiscordOverlayManager* manager,void* on_ime_composition_range_changed_data,	// 0x0080
			void (*on_ime_composition_range_changed)(void* on_ime_composition_range_changed_data, int from, int to,
				DiscordRect* bounds, std::uint32_t bounds_length));
		void (*set_ime_selection_bounds_callback)(IDiscordOverlayManager* manager, void* on_ime_selection_bounds_changed_data,	// 0x0088
			void (*on_ime_selection_bounds_changed)(void* on_ime_selection_bounds_changed_data, DiscordRect anchor,
				DiscordRect focus, bool is_anchor_first));
		bool (*is_point_inside_click_zone)(IDiscordOverlayManager* manager, int x, int y);										// 0x0090
	};
	ENGINE_ASSERT_SZ(IDiscordOverlayManager, 0x0098);
}

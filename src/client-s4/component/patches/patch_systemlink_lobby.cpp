#include "common.hpp"
#include "game/dvars.hpp"
#include "game/game.hpp"

#include <component/scheduler.hpp>
#include <loader/component_loader.hpp>
#include <utils/hook.hpp>
#include <engine/s4/PartyData.hpp>

namespace patches::systemlink_lobby {
	namespace {
		utils::hook::detour live_get_game_party_hook;
	}

	bool is_in_lobby() {
		return game::Live_IsInSystemlinkLobby();
	}

	s4::PartyData* live_get_game_party_stub() {
		if (game::Live_IsInSystemlinkLobby()) {
			return game::g_partyData;
		}
		return live_get_game_party_hook.invoke<s4::PartyData*>(); // patch bots spawning
	}

	struct component final : generic_component {
		bool should_activate() override {
			return game::Live_IsInSystemlinkLobby != nullptr;
		}

		void post_unpack() override {
			live_get_game_party_hook.create(game::Live_GetGameParty, live_get_game_party_stub);

			scheduler::loop<scheduler::pipeline::client>([] {
				static std::optional<bool> current_systemlink_lobby_value{};
				static s4::dvar_t* systemlink = nullptr;
				if (systemlink == nullptr) {
					systemlink = dvars::get_dvar_by_name("systemlink");
				}

				bool systemlink_cur = dvars::get_enabled(systemlink);
				if (!current_systemlink_lobby_value.has_value() || systemlink_cur != current_systemlink_lobby_value.value()) {
					std::uint8_t func_stub[] = {
						0x00, 0x00,	// ?? ??
						0xC3		// retn
					};
					if (systemlink_cur) {
						// mov al, 1
						func_stub[0] = 0xB0;
						func_stub[1] = 0x01;
					}
					else {
						// xor al, al
						func_stub[0] = 0x32;
						func_stub[1] = 0xC0;
					}
					utils::hook::copy(game::Live_IsInSystemlinkLobby, func_stub, sizeof(func_stub));
					current_systemlink_lobby_value = systemlink_cur;
				}
			});
		}
	};
}

REGISTER_COMPONENT(patches::systemlink_lobby::component)
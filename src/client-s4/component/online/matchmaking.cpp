#include "common.hpp"
#include "component/online/matchmaking.hpp"
#include "component/online/matchmaking_state.hpp"

#include "component/command.hpp"
#include "component/lua/lua_hook.hpp"
#include "component/lua/lua_vm.hpp"
#include "game/game.hpp"

#include <component/scheduler.hpp>
#include <engine/s4/MatchmakerUIStateData.hpp>
#include <loader/component_loader.hpp>
#include <utils/hook.hpp>

namespace online::matchmaking { // basically this whole matchmaking thing is a paste of client-iw8
	namespace {
		using get_ui_state_t = s4::MatchmakerUIState(void*, s4::MatchmakerUIStateData*);
		using start_check_t = bool(void*);

		get_ui_state_t* OnlineMatchmakerOmniscient__GetUIState{};
		start_check_t* unk_matchmaker_start_gate{};
		start_check_t* unk_matchmaker_start_ack{};
		void* matchmaker_start_gate_call{};
		void* matchmaker_start_ack_call{};

		utils::hook::detour get_ui_state_hook;
		utils::hook::detour is_in_dw_lobby_hook;
		ui_state_override state_override;
		std::atomic<bool> start_requested = false;

		bool native_ready() {
			return OnlineMatchmakerOmniscient__GetUIState && unk_matchmaker_start_gate && unk_matchmaker_start_ack
				&& matchmaker_start_gate_call && matchmaker_start_ack_call;
		}

		bool in_frontend() {
			return game::Com_FrontEnd_IsInFrontEnd && game::Com_FrontEnd_IsInFrontEnd();
		}

		s4::PartyData* get_running_lobby() {
			if (!game::Live_GetGameParty || !game::Party_IsRunning) {
				return nullptr;
			}
			auto* lobby = game::Live_GetGameParty();
			return lobby && game::Party_IsRunning(lobby) != 0 ? lobby : nullptr;
		}

		bool can_force_start() {
			if (!in_frontend() || !game::Party_AreWeServer) {
				return false;
			}

			const auto* lobby = get_running_lobby();
			return lobby && game::Party_AreWeServer(lobby);
		}

		s4::MatchmakerUIState get_ui_state_stub(void* matchmaker, s4::MatchmakerUIStateData* extra_info) {
			const auto override = state_override.get();
			if (!override || !in_frontend()) {
				return get_ui_state_hook.invoke<s4::MatchmakerUIState>(matchmaker, extra_info);
			}

			if (extra_info) {
				extra_info->max_ping_ = override->extra_info;
			}

			return override->state;
		}

		int is_in_dw_lobby_stub(s4::lua_State* lua_vm) {
			const auto override = state_override.get();

			if (!override || !in_frontend()) {
				return is_in_dw_lobby_hook.invoke<int>(lua_vm);
			}

			const auto searching = override->state == s4::MatchmakerUIState::SEARCHING
				|| override->state == s4::MatchmakerUIState::JOINED_LOBBY;
			lua::vm::push_boolean(lua_vm, searching && get_running_lobby() != nullptr);
			return 1;
		}

		bool start_gate_stub(void* matchmaker) {
			return (start_requested && can_force_start()) || unk_matchmaker_start_gate(matchmaker);
		}

		bool start_ack_stub(void* matchmaker) {
			return (start_requested && can_force_start()) || unk_matchmaker_start_ack(matchmaker);
		}

		void set_state_command(const command::params_cl& params) {
			if (params.size() < 2 || params.size() > 3) {
				LOG("Component/Online/Matchmaking", INFO, "Usage: mm_setMMState <0-5> <max_ping|qos_players>");
				return;
			}

			if (params[1] == "native" && params.size() == 2) {
				reset();
				return;
			}

			const auto state = parse_integer(params[1]);
			const auto extra_info = params.size() == 3 ? parse_integer(params[2]) : std::optional<int>{ 0 };

			if (!state || !extra_info || !set_state(static_cast<s4::MatchmakerUIState>(*state), *extra_info)) {
				LOG("Component/Online/Matchmaking", WARN, "Bad usage of mm_setMMState");
			}
		}

		void start_match(bool private_match) {
			if (!native_ready() || !can_force_start() || !game::GScr_EndLobby || !game::GScr_EndLobby_ConBuf) {
				LOG("Component/Online/Matchmaking", WARN, "Start the experimental match from a lobby you host in the frontend");
				return;
			}

			if (identification::game::is_config(identification::game::Beta)) {
				LOG("Component/Online/Matchmaking", WARN, "Beta dvar hashing isn't verified yet, leaving this on the todo list..");
				return;
			}

			auto commands = std::format("#x3{:X} 1;#x3{:X} 1;",
				game::hash_dvar("party_minplayers"), game::hash_dvar("live_lobby_minplayers_start"));

			if (private_match) {
				commands += std::format("#x3{:X} 1;xpartygo;", game::hash_dvar("xblive_privatematch"));
			}
			
			start_requested = true;
			game::Cbuf_AddText(commands + "\n");
			LOG("Component/Online/Matchmaking", INFO, "Requested hosted {} match start", private_match ? "private" : "public");
		}

		void update() {
			if (!in_frontend() || (start_requested && !can_force_start())) {
				reset();
			}
		}
	}

	bool supports_match_start() {
		return native_ready() && game::GScr_EndLobby && game::GScr_EndLobby_ConBuf
			&& !identification::game::is_config(identification::game::Beta);
	}

	bool set_state(s4::MatchmakerUIState state, int extra_info) {
		return native_ready() && in_frontend() && state_override.set(state, extra_info);
	}

	void reset() {
		state_override.reset();
		start_requested = false;
	}

	struct component final : generic_component {
		void find_signatures(memory::signature_store& batch) override {
			batch.add(SETUP_POINTER(OnlineMatchmakerOmniscient__GetUIState), "48 89 5C 24 ? 57 48 83 EC ? 80 B9 ? ? ? ? ? 48 8B FA 48 8B D9 74 ? 8B 81");

			batch.add(SETUP_POINTER(unk_matchmaker_start_gate), "48 8D 0D ? ? ? ? E8 ? ? ? ? 84 C0 0F 85 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 84 C0 0F 84", SETUP_MOD(add(8).rip()));

			batch.add(SETUP_POINTER(unk_matchmaker_start_ack), "48 8D 0D ? ? ? ? E8 ? ? ? ? 84 C0 0F 85 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 84 C0 0F 84", SETUP_MOD(add(28).rip()));

			batch.add(SETUP_POINTER(matchmaker_start_gate_call), "48 8D 0D ? ? ? ? E8 ? ? ? ? 84 C0 0F 85 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 84 C0 0F 84", SETUP_MOD(add(7)));

			batch.add(SETUP_POINTER(matchmaker_start_ack_call), "48 8D 0D ? ? ? ? E8 ? ? ? ? 84 C0 0F 85 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 84 C0 0F 84", SETUP_MOD(add(27)));
		}

		void post_load() override {
			lua::hook::create(is_in_dw_lobby_hook, "Lobby.IsInDWLobby", is_in_dw_lobby_stub);
		}

		void post_unpack() override {
			if (!native_ready()) {
				LOG("Component/Online/Matchmaking", INFO, "Experimental matchmaking unavailable for this build; keeping native behavior");
				return;
			}

			get_ui_state_hook.create(OnlineMatchmakerOmniscient__GetUIState, get_ui_state_stub);
			utils::hook::call(matchmaker_start_gate_call, start_gate_stub);
			utils::hook::call(matchmaker_start_ack_call, start_ack_stub);

			command::add("mm_setMMState", set_state_command);
			command::add("mm_forceStartPrivateMatch", [] { start_match(true); });
			command::add("mm_startMatch", [] { start_match(false); });
			command::add("mm_reset", reset);
			scheduler::loop<scheduler::pipeline::client>(update);
		}
	};
}

#if !defined(CLIENT_F2P) && !defined(CLIENT_OFFLINE)
REGISTER_COMPONENT(online::matchmaking::component)
#endif
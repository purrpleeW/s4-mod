#include "common.hpp"
#include "component/patches/patch_beta_auth.hpp"

#include "component/auth.hpp"
#include "component/lua/lua_hook.hpp"
#include "component/lua/lua_vm.hpp"

#include <utils/hook.hpp>

/*
	This is some of the stupidest shit i've ever seen in my life,
	the beta auth system is incredibly different from ship, and I had to
	get the help of claude to help reverse this stupid mess - take a look at 
	some of this in IDA.

	~ purrplee
*/

namespace patches::beta_auth {
	namespace {
		functions::GamerProfile_LogInProfileT* GamerProfile_LogInProfile{};
		functions::CL_PlayerData_GetDDLBufferT* CL_PlayerData_GetDDLBuffer{};
		void (*unk_bnet_connection_frame)(s4::CoDAuthState*){};
		void (*unk_bnet_profanity_begin_fetch)(void*){};
		void (*unk_demonware_pump)(void*){};
		bool (*unk_platform_auth_is_ready)(){};
		void (*unk_cl_activate_client)(s4::LocalClientNum_t, int){};
		bool (*unk_cl_is_controller_mapped_to_client)(int, s4::LocalClientNum_t*){};

		utils::hook::detour controller_active_hook;
		std::atomic<bool> client_activation_requested{ false };

		bool stats_buffers_ready() { // beta only has 8 DDL stats groups
			for (int group = 0; group < 8; ++group) {
				s4::DDLContext context{};
				if (!CL_PlayerData_GetDDLBuffer(&context, 0, s4::StatsSource::STATS_ONLINE,
					static_cast<s4::StatsGroup>(group))) {
					return false;
				}
			}
			return true;
		}

		int is_controller_active_stub(s4::lua_State* lua_vm) {
			const auto controller_index = lua::vm::get_integer(lua_vm, 1);
			const auto* user_data = auth::get_local_user_data();
			if (unk_cl_activate_client && unk_cl_is_controller_mapped_to_client && controller_index == 0
				&& user_data && user_data->sign_in_state_ == s4::CoDAuthState::AUTHSTATE_SUCCESS
				&& !unk_cl_is_controller_mapped_to_client(controller_index, nullptr)) {
				client_activation_requested = true;
			}
			return controller_active_hook.invoke<int>(lua_vm);
		}

		void unk_bnet_connection_frame_stub(s4::CoDAuthState* state) {
			*state = s4::CoDAuthState::AUTHSTATE_SUCCESS;
		}

		void unk_bnet_profanity_begin_fetch_stub(void* service) {
			CLASS_VALUE_AT(service, 0x40, bool) = false;
			CLASS_VALUE_AT(service, 0x41, bool) = false;
		}

		void unk_demonware_pump_stub(void*) {}

		bool return_true_stub() {
			return true;
		}
	}

	void post_load() {
		lua::hook::create(controller_active_hook, "Engine.BBHAECABBD", is_controller_active_stub);
	}

	void find_signatures(memory::signature_store& batch,
		functions::PlatformAuth_GetUsernameT*& get_username, functions::PlatformAuth_GetXUIDT*& get_xuid) {

		batch.add(SETUP_POINTER(GamerProfile_LogInProfile), "4C 8B DC 49 89 5B ? 55 56 41 54 41 55 41 56");

		batch.add(SETUP_POINTER(CL_PlayerData_GetDDLBuffer), "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B E9 49 63 F8");

		batch.add(SETUP_POINTER(unk_cl_activate_client), "48 89 5C 24 08 57 48 83 EC 20 48 63 FA 8B D9 E8 ? ? ? ? B2 01 8B CB E8");
		
		batch.add(SETUP_POINTER(unk_cl_is_controller_mapped_to_client), "83 3D ? ? ? ? 00 74 ? 85 C9 75 ? 48 63 C1 48 8D 0D ? ? ? ? 8B 0C 81 48 85 D2");
		
		// auth frame reports BLZBNTBGS00000BD3 if not patched
		batch.add(SETUP_POINTER(unk_platform_auth_is_ready), "E8 ? ? ? ? 84 C0 0F 85 ? ? ? ? 33 C9 E8 ? ? ? ? 85 C0 7E", SETUP_MOD(add(1).rip()));
		
		batch.add(SETUP_POINTER(get_username), "E8 ? ? ? ? 84 C0 75 ? C5 F9 EF C0 33 C0 C5 FC 11 44 24", SETUP_MOD(add(1).rip()));
		
		batch.add(SETUP_POINTER(get_xuid), "E8 ? ? ? ? 84 C0 75 ? 48 8D 15 ? ? ? ? B9 ? ? ? ? E8 ? ? ? ? 48 8B 5C 24", SETUP_MOD(add(1).rip()));
	}

	void find_signatures_bnet(memory::signature_store& batch) {
		batch.add(SETUP_POINTER(unk_bnet_connection_frame), "48 83 EC ? E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? " "E8 ? ? ? ? 48 8B C8 E8", SETUP_MOD(add(13).rip()));
		
		batch.add(SETUP_POINTER(unk_bnet_profanity_begin_fetch), "48 89 5C 24 ? 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 44 24 ? "
			"48 83 79 48 00 48 8B F9 C6 41 41 00");
		
		batch.add(SETUP_POINTER(unk_demonware_pump), "40 53 B8 70 33 01 00 E8 ? ? ? ? 48 2B E0 48 8B 05 ? ? ? ? 48 33 C4 "
			"48 89 84 24 ? ? ? ? 48 8B D9 48 8B 49 10");
	}

	bool should_activate() {
		return GamerProfile_LogInProfile && CL_PlayerData_GetDDLBuffer && unk_platform_auth_is_ready
			&& unk_cl_activate_client && unk_cl_is_controller_mapped_to_client;
	}

	bool should_activate_bnet() {
		return unk_bnet_connection_frame && unk_bnet_profanity_begin_fetch && unk_demonware_pump;
	}

	void post_unpack() {
		utils::hook::jump(unk_platform_auth_is_ready, return_true_stub);

		if (identification::game::is_platform(identification::game::BattleNet)) {
			utils::hook::jump(unk_bnet_connection_frame, unk_bnet_connection_frame_stub);
			utils::hook::jump(unk_bnet_profanity_begin_fetch, unk_bnet_profanity_begin_fetch_stub);
			utils::hook::jump(unk_demonware_pump, unk_demonware_pump_stub);
		}
	}

	void finish_auth(s4::BattleNetAuth* auth) {
		auth->finished_auth_beta_ = true;
	}

	void sign_in_profile() {
		GamerProfile_LogInProfile(0);
	}

	void frame(void (*initialize_stats)()) {
		static bool stats_initialized = false;
		if (!stats_initialized && stats_buffers_ready()) {
			initialize_stats();
			stats_initialized = true;
		}

		if (stats_initialized && client_activation_requested.exchange(false)) {
			if (!unk_cl_is_controller_mapped_to_client(0, nullptr)) {
				unk_cl_activate_client(s4::LocalClientNum_t::LOCAL_CLIENT_0, 0);
			}
		}
	}
}

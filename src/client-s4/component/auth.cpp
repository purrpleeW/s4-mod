#include "common.hpp"
#include "component/auth.hpp"

#include "component/lua/lua_hook.hpp"
#include "component/patches/patch_beta_auth.hpp"

#include "component/name.hpp"
#include "game/game.hpp"

#include <loader/component_loader.hpp>
#include <utils/concurrency.hpp>
#include <utils/cryptography.hpp>
#include <utils/hook.hpp>
#include <utils/storage/storage_immutable.hpp>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

namespace auth {
	namespace {
		functions::BattleNetAuth_BeginAuthT* BattleNetAuth_BeginAuth{};
		// functions::dwGetLogOnStatusT* dwGetLogOnStatus{};
		functions::Live_CanUserPlayOnlineT* Live_CanUserPlayOnline{};
		functions::Live_UserSignedInToLiveT* Live_UserSignedInToLive{};
		// functions::Live_IsUserSignedInToDemonwareT* Live_IsUserSignedInToDemonware{};
		functions::LiveStorage_GetActiveStatsSourceT* LiveStorage_GetActiveStatsSource{};
		functions::LiveStorage_ReadStatsT* LiveStorage_ReadStats{};
		functions::LiveStorage_StatsInitT* LiveStorage_StatsInit{};
		functions::LocalUserData_GetByControllerT* LocalUserData_GetByController{};
		functions::PlatformAuth_FrameT* PlatformAuth_Frame{};
		functions::PlatformAuth_GetUsernameT* PlatformAuth_GetUsername{};
		functions::PlatformAuth_GetXUIDT* PlatformAuth_GetXUID{};

		functions::Live_IsConnectedToDemonwareT* Live_IsConnectedToDemonware{};


		void* BattleNetAuth_LoadRegistry_Call{};
		void* BattleNetAuth_ValidateRegion_Call{};
		s4::offset_PlayerStats* offset_controllerStatData_playerStats{};
		std::uint32_t* offset_controllerStatData_playerStats_unk_fetched{};
		bool* s_isContentEnumerationFinished{};

		utils::hook::detour dw_get_log_on_status_hook;
		//utils::hook::detour live_is_user_signed_in_to_demonware_hook;
		utils::hook::detour platform_auth_frame_hook;

		std::vector<std::function<void()>> stats_init_callbacks{};
		utils::concurrency::container<std::string> xuid{};

		void activate_xuid(std::string new_xuid) {
			xuid.access([&](std::string& xuid_str) {
				xuid_str = std::move(new_xuid);
			});
		}

		void setup_xuid() {
			user_xuid.id_ = utils::cryptography::random::get_big_integer();
			auto _ = utils::storage::immutable_storage.set("xuid", user_xuid.to_hex_string());
			activate_xuid(user_xuid.to_hex_string());
		}

		void load_xuid() {
			const auto xuid_storage = utils::storage::immutable_storage.get("xuid");
			xuid_storage.has_value() ? activate_xuid(xuid_storage.value()) : setup_xuid();
		}

		bool live_can_user_play_online_stub(int controller_index, std::int32_t* failure_reason) {
			*failure_reason = 0 /*success*/;
			return true;
		}

		s4::StatsSource live_storage_get_active_stats_source_stub(int controller_index) {
			return s4::StatsSource::STATS_ONLINE;
		}

		void battle_net_auth_begin_auth_stub(s4::BattleNetAuth* _this) {
			_this->state_ = s4::CoDAuthState::AUTHSTATE_SUCCESS;

			if (identification::game::is_config(identification::game::Beta)) {
				patches::beta_auth::finish_auth(_this);
			}
			else {
				_this->finished_auth_ = true;
				_this->var3_ = 0x795230F0;
				_this->var4_ = 0x1F;
				_this->var5_ = 0x00000000;
			}
		}

		void initialize_stats() {
			LiveStorage_StatsInit(0, true, false, s4::StatsSource::STATS_ONLINE);
			for (auto& callback : stats_init_callbacks) {
				callback();
			}
		}

		void platform_auth_frame_stub() {
			const auto is_beta = identification::game::is_config(identification::game::Beta);
			static std::once_flag auth_frame_init;
			std::call_once(auth_frame_init, [] {
				if (!identification::game::is_config(identification::game::Beta)){
					char* disconnect_message = nullptr;
					if (Live_UserSignedInToLive(0, &disconnect_message)) {
						LOG("Component/Auth", WARN, "Failed to sign in: {}", game::SEH_StringEd_GetString(disconnect_message));
					}
				}

				// Local User Data setup
				{
					auto user_data = get_local_user_data();
					user_data->assign_username(name::get_player_name());
					user_data->assign_xuid(user_xuid.get_universal_id());
					user_data->sign_in_state_ = s4::CoDAuthState::AUTHSTATE_SUCCESS;
				}

				// this is my worst enemy, this mf got me chasing ghosts on vg beta
				if (s_isContentEnumerationFinished) {
					*s_isContentEnumerationFinished = true;
				}

				// Player stats setup, why does vanguard torture me like this...
				{
					offset_controllerStatData_playerStats->stats_fetched_ = true;
					offset_controllerStatData_playerStats->online_stat_write_pending_ = false;
					offset_controllerStatData_playerStats->allow_saving_progress_ = true;

					CLASS_VALUE_AT(offset_controllerStatData_playerStats, *offset_controllerStatData_playerStats_unk_fetched, bool) = true;
				}

				// Profile sign-in (Beta)
				if (identification::game::is_config(identification::game::Beta)) {
					patches::beta_auth::sign_in_profile();
				}

				// Player stats part, the easy part
				{
					if (!identification::game::is_config(identification::game::Beta)) {
						initialize_stats();
					}
				}
			});

			platform_auth_frame_hook.invoke<void>();
			if (is_beta) {
				patches::beta_auth::frame(initialize_stats);
			}
		}

		bool platform_auth_get_username_stub(int controller_index, char* buffer_out, std::size_t buffer_size) {
			if (!buffer_out) {
				return false;
			}

			utils::string::copy(buffer_out, buffer_size, name::get_player_name());
			return true;
		}

		bool platform_auth_get_xuid_stub(int controller_index, s4::XUID* xuid_out) {
			if (!xuid_out) {
				return false;
			}

			*xuid_out = user_xuid;
			return true;
		}

		bool scenarios_platform_is_scenario_installed_stub(s4::Scenario* scenario) {
			return true;
		}

		void sub_357ae10_stub(int* a1) {
			if (!a1) {
				return;
			}
			*a1 = 0;
		}

		bool return_true_stub() {
			return true;
		}
	}

	void on_stats_init(std::function<void()> callback) {
		stats_init_callbacks.push_back(callback);
	}

	s4::LocalUserData* get_local_user_data() {
		if (LocalUserData_GetByController) {
			return LocalUserData_GetByController(0);
		}
		return nullptr;
	}

	struct component final : generic_component {
		void post_load() override {
			load_xuid();
			user_xuid.from_hex_string(xuid.copy().c_str());

			lua::hook::set("Engine.IsLocalPlayAllowed", true);
			lua::hook::set("Engine.IsPremiumPlayer", true);
			lua::hook::set("Fences.OfflineDataFetched", true);
			if (identification::game::is_config(identification::game::Beta)) {
				patches::beta_auth::post_load();
			}
		}

		void find_signatures(memory::signature_store& batch) override {
			// batch.add(SETUP_POINTER(dwGetLogOnStatus), "40 53 48 83 EC ? 48 63 C1 BA ? ? ? ? 48 69 D8");

			if (identification::game::is_config(identification::game::Beta)) {
				patches::beta_auth::find_signatures(batch, PlatformAuth_GetUsername, PlatformAuth_GetXUID);
			}

			batch.add(SETUP_POINTER(Live_CanUserPlayOnline), "E8 ? ? ? ? 0F B6 D0 48 8B CB E8 ? ? ? ? 48 63 54 24", SETUP_MOD(add(1).rip()));

			if (!identification::game::is_config(identification::game::Beta)) {
				batch.add(SETUP_POINTER(Live_UserSignedInToLive), "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F2 8B D9 E8");
			}

			// batch.add(SETUP_POINTER(Live_IsUserSignedInToDemonware), "40 53 48 83 EC ? 8B D9 E8 ? ? ? ? 80 B8");

			batch.add(SETUP_POINTER(Live_IsConnectedToDemonware), "E8 ? ? ? ? 84 C0 74 ? 8B D3 48 8D 4C 24 ? E8 ? ? ? ? 48 8B C8", SETUP_MOD(add(1).rip()));

			batch.add(SETUP_POINTER(LiveStorage_StatsInit), "44 89 4C 24 ? 44 88 44 24 ? 88 54 24 ? 89 4C 24 ? 53");

			batch.add(SETUP_POINTER(LocalUserData_GetByController), "48 83 EC ? 80 3D ? ? ? ? ? 74 ? 8B D1");

			if (!identification::game::is_config(identification::game::Beta)) {
				batch.add(SETUP_POINTER(PlatformAuth_GetUsername), "E8 ? ? ? ? 84 C0 74 ? B0 ? 48 8B 5C 24 ? 48 83 C4 ? 5F C3 4C 8B C7 33 D2", SETUP_MOD(add(1).rip()));

				if (identification::game::is_greater_or_eq("1.27.0")) {
					batch.add(SETUP_POINTER(PlatformAuth_GetXUID), "E8 ? ? ? ? 84 C0 75 ? 38 05 ? ? ? ? 74 ? 33 D2 F7 05 ? ? ? ? ? ? ? ? 76 ? 4C 8D 05 ? ? ? ? 0F 1F 40 ? 66 0F 1F 84 00 ? ? ? ? 8B C2 ? ? ? ? 83 E0", SETUP_MOD(add(1).rip()));
				}
				else {
					batch.add(SETUP_POINTER(PlatformAuth_GetXUID), "E8 ? ? ? ? 84 C0 75 ? 38 05 ? ? ? ? 74 ? 33 D2 F7 05 ? ? ? ? ? ? ? ? 76 ? 4C 8D 05 ? ? ? ? 0F 1F "
						"40 ? 8B C2 4A 8D 0C 02", SETUP_MOD(add(1).rip()));
				}
			}

			batch.add(SETUP_POINTER(offset_controllerStatData_playerStats_unk_fetched), "4C 69 ? ? ? ? ? 4B 8D 2C 3C", SETUP_MOD(add(3)));

			batch.add(SETUP_POINTER(offset_controllerStatData_playerStats), "80 BC 2E ? ? ? ? ? 74 ? 8B CB", SETUP_MOD(add(3).based_rel()));

			batch.add(SETUP_POINTER(s_isContentEnumerationFinished), "80 3D ? ? ? ? ? 74 ? 48 89 7C 24 ? E8 ? ? ? ? 85 C0", SETUP_MOD(add(2).rip().add(1)));
		}

		void find_signatures_bnet(memory::signature_store& batch) override {
			if (identification::game::is_config(identification::game::Beta)) {
				patches::beta_auth::find_signatures_bnet(batch);
			}

			batch.add(SETUP_POINTER(BattleNetAuth_BeginAuth), "E8 ? ? ? ? E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? E8 ? ? ? ? 84 C0",
				SETUP_MOD(add(1).rip()));

			batch.add(SETUP_POINTER(BattleNetAuth_ValidateRegion_Call), "E8 ? ? ? ? 8B 0D ? ? ? ? 8B 15 ? ? ? ? 48 83 C4");

			if (identification::game::is_greater_or_eq("1.16.0")) {
				batch.add(SETUP_POINTER(BattleNetAuth_LoadRegistry_Call), "E8 ? ? ? ? 48 8D 15 ? ? ? ? B1 ? 48 8B 5C 24");
			}
			else {
				batch.add(SETUP_POINTER(BattleNetAuth_LoadRegistry_Call), "E8 ? ? ? ? B1 ? 48 8B 5C 24 ? 48 83 C4");
			}

			batch.add(SETUP_POINTER(PlatformAuth_Frame), "48 83 EC ? E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? E8 ? ? ? ? 48 8B C8 E8 ? ? ? ? E8 ? ? ? ? 48 8B C8 E8");
		}

		void find_signatures_steam(memory::signature_store& batch) override {
			batch.add(SETUP_POINTER(LiveStorage_GetActiveStatsSource), "40 56 48 83 EC ? 48 63 C1 48 8D 35");

			batch.add(SETUP_POINTER(LiveStorage_ReadStats), "41 56 48 81 EC ? ? ? ? 4C 63 F1 B9");

			batch.add(SETUP_POINTER(PlatformAuth_Frame), "48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 80 3D ? ? ? ? ? 74");
		}

		bool should_activate() override {
			const auto has_platform_auth = Live_CanUserPlayOnline && LiveStorage_StatsInit && PlatformAuth_Frame
				&& LocalUserData_GetByController && PlatformAuth_GetUsername && PlatformAuth_GetXUID
				&& Live_IsConnectedToDemonware && offset_controllerStatData_playerStats_unk_fetched
				&& offset_controllerStatData_playerStats;

			if (identification::game::is_config(identification::game::Beta)) {
				return has_platform_auth && patches::beta_auth::should_activate();
			}

			return has_platform_auth && Live_UserSignedInToLive;
		}

		bool should_activate_bnet() override {
			const auto has_bnet_auth = BattleNetAuth_BeginAuth && BattleNetAuth_ValidateRegion_Call
				&& BattleNetAuth_LoadRegistry_Call;

			return has_bnet_auth && (!identification::game::is_config(identification::game::Beta)
				|| patches::beta_auth::should_activate_bnet());
		}

		bool should_activate_steam() override {
			return LiveStorage_GetActiveStatsSource && LiveStorage_ReadStats;
		}

		void post_unpack() override {
			if (identification::game::is_platform(identification::game::Steam)) {
				utils::hook::jump(LiveStorage_GetActiveStatsSource, live_storage_get_active_stats_source_stub);
				utils::hook::set<std::uint8_t>(LiveStorage_ReadStats, 0xC3);
			}

			platform_auth_frame_hook.create(PlatformAuth_Frame, platform_auth_frame_stub);

			utils::hook::jump(Live_CanUserPlayOnline, live_can_user_play_online_stub);
			utils::hook::jump(PlatformAuth_GetUsername, platform_auth_get_username_stub);
			utils::hook::jump(PlatformAuth_GetXUID, platform_auth_get_xuid_stub);

			utils::hook::jump(Live_IsConnectedToDemonware, return_true_stub);

			if (identification::game::is_config(identification::game::Beta)) {
				patches::beta_auth::post_unpack();
			}

			if (identification::game::is_platform(identification::game::BattleNet)) {
				utils::hook::jump(BattleNetAuth_BeginAuth, battle_net_auth_begin_auth_stub);

				utils::hook::nop(BattleNetAuth_LoadRegistry_Call, 5);
				utils::hook::nop(BattleNetAuth_ValidateRegion_Call, 5);
			}

			// utils::hook::jump(0x357AE10_b, sub_357ae10_stub);

			// utils::hook::nop(0x3578f25_b, 2);

			// test_dtr.create(0x3578e40_b, test_stub);

			// utils::hook::jump(dwGetLogOnStatus, dw_get_log_on_status_stub);
			// utils::hook::jump(Live_IsUserSignedInToDemonware, live_is_user_signed_into_demonware_stub);
		}
	};
}

REGISTER_COMPONENT(auth::component)

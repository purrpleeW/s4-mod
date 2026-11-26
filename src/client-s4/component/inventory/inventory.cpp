#include "common.hpp"

#include "component/auth.hpp"
#include "component/command.hpp"
#include "component/lua/lua_hook.hpp"
#include "component/scheduler.hpp"
#include "game/dvars.hpp"
#include "game/game.hpp"
#include "identification/game.hpp"
#include "utils/string.hpp"
#include "component/lua/lua_hook.hpp"
#include "component/lua/lua_vm.hpp"

#include <loader/component_loader.hpp>
#include <utils/hook.hpp>
#include <utils/properties.hpp>
#include <utils/storage/storage_immutable.hpp>


namespace inventory {
	namespace {
		functions::CL_PlayerData_GetDDLBufferT* CL_PlayerData_GetDDLBuffer{};
		functions::DDL_LocallyObfuscateT* DDL_LocallyObfuscate{};
		functions::DDL_LocallyUnObfuscateT* DDL_LocallyUnObfuscate{};
		functions::Online_Loot_GetItemQuantityT* Online_Loot_GetItemQuantity{};

		utils::hook::detour lui_cod_lua_call_notify_server_hook;

		std::vector<std::tuple<s4::StatsGroup, std::string, std::string, std::string>> stat_groups = {
			{ s4::StatsGroup::STATSGROUP_RANKED, "STATSGROUP_RANKED", "ranked", "ranked stats" },
			{ s4::StatsGroup::STATSGROUP_COOP, "STATSGROUP_COOP", "coop", "co-op inventory" },
			{ s4::StatsGroup::STATSGROUP_COMMON, "STATSGROUP_COMMON", "common", "common inventory" },
			{ s4::StatsGroup::STATSGROUP_RANKEDLOADOUTS, "STATSGROUP_RANKEDLOADOUTS", "ranked_loadouts", "ranked loadouts" },
			{ s4::StatsGroup::STATSGROUP_PRIVATELOADOUTS, "STATSGROUP_PRIVATELOADOUTS", "private_loadouts", "private loadouts" },
			{ s4::StatsGroup::STATSGROUP_WZRANKEDLOADOUTS, "STATSGROUP_WZRANKEDLOADOUTS", "wz_ranked_loadouts", "Warzone ranked loadouts" },
			{ s4::StatsGroup::STATSGROUP_WZPRIVATELOADOUTS, "STATSGROUP_WZPRIVATELOADOUTS", "wz_private_loadouts", "Warzone private loadouts" },
			{ s4::StatsGroup::STATSGROUP_LEAGUEPLAYLOADOUTS, "STATSGROUP_LEAGUEPLAYLOADOUTS", "lp_loadouts", "League Play loadouts" },
			{ s4::StatsGroup::STATSGROUP_LEAGUEPLAYPROGRESSION, "STATSGROUP_LEAGUEPLAYPROGRESSION", "lp_progression", "League Play progression" },
			{ s4::StatsGroup::STATSGROUP_NONGAME, "STATSGROUP_NONGAME", "non_game", "customizations" },
			{ s4::StatsGroup::STATSGROUP_ZMLOADOUTS, "STATSGROUP_ZMLOADOUTS", "zm_loadouts", "Zombies loadouts" },
		};

		std::optional<s4::StatsGroup> get_stats_group_for_build(s4::StatsGroup group) {
			if (!identification::game::is_config(identification::game::Beta)) {
				return group;
			}
			if (group == s4::StatsGroup::STATSGROUP_NONGAME) {
				return static_cast<s4::StatsGroup>(7);
			}
			if (group >= s4::StatsGroup::STATSGROUP_LEAGUEPLAYLOADOUTS) {
				return std::nullopt;
			}
			return group;
		}

		std::filesystem::path get_settings_file_bin(const std::string& section) {
			std::filesystem::path inventory_folder = utils::properties::get_appdata_path() / "inventory" / std::format("v{}",
				identification::utility::get_game_version());
			std::filesystem::create_directories(inventory_folder);

			return inventory_folder / (section + ".bin");
		}

		void load_ddl_context(s4::DDLContext* ctx, const std::string& section) {
			bool was_obfuscated = ctx->obfuscated_;

			if (was_obfuscated) {
				DDL_LocallyUnObfuscate(ctx, static_cast<std::uint8_t*>(ctx->buff_), ctx->len_);
			}

			std::ifstream file(get_settings_file_bin(section), std::ios::binary | std::ios::ate);
			if (file) {
				std::streamsize size = file.tellg();
				file.seekg(0, std::ios::beg);

				if (size > 0) {
					if (ctx->len_ != static_cast<int>(size)) {
						LOG("Component/Inventory", WARN, "DDLContext size mismatch on section \"{}\" (cl:{},fi:{})", section,
							ctx->len_, static_cast<int>(size));
					}

					if (!file.read(static_cast<char*>(ctx->buff_), size)) {
						LOG("Component/Inventory", WARN, "Failed to read DDLContext on section \"{}\"", section);
					}
				}
			}

			if (was_obfuscated) {
				DDL_LocallyObfuscate(ctx, static_cast<std::uint8_t*>(ctx->buff_), ctx->len_, ctx->random_int_);
			}
		}

		void save_ddl_context(s4::DDLContext* ctx, const std::string& section) {
			bool was_obfuscated = ctx->obfuscated_;

			if (was_obfuscated) {
				DDL_LocallyUnObfuscate(ctx, static_cast<std::uint8_t*>(ctx->buff_), ctx->len_);
			}

			std::ofstream file(get_settings_file_bin(section), std::ios::binary);
			if (file) {
				file.write(static_cast<char*>(ctx->buff_), ctx->len_);
			}

			if (was_obfuscated) {
				DDL_LocallyObfuscate(ctx, static_cast<std::uint8_t*>(ctx->buff_), ctx->len_, ctx->random_int_);
			}
		}

		void load() {
			s4::DDLContext context;
			for (auto& [group, group_name, file_name, display_name] : stat_groups) {
				const auto build_group = get_stats_group_for_build(group);
				if (!build_group) {
					continue;
				}
				if (CL_PlayerData_GetDDLBuffer(&context, 0, s4::StatsSource::STATS_ONLINE, *build_group)) {
					load_ddl_context(&context, file_name);
					LOG("Component/Inventory", INFO, "Loaded {}.", display_name);
				}
				else {
					LOG("Component/Inventory", WARN, "Couldn't get DDLBuffer for {}, called before initialized?", group_name);
				}
			}
		}

		void save() {
			s4::DDLContext context;
			for (auto& [group, group_name, file_name, display_name] : stat_groups) {
				const auto build_group = get_stats_group_for_build(group);
				if (!build_group) {
					continue;
				}
				if (CL_PlayerData_GetDDLBuffer(&context, 0, s4::StatsSource::STATS_ONLINE, *build_group)) {
					save_ddl_context(&context, file_name);
					LOG("Component/Inventory", INFO, "Saved {}.", display_name);
				}
				else {
					LOG("Component/Inventory", WARN, "Couldn't get DDLBuffer for {}, called before initialized?", group_name);
				}
			}
		}

		int lui_cod_lua_call_notify_server_stub(s4::lua_State* lua_vm) {
			std::string channel_name = lua::vm::get_std_string(lua_vm, 1);
			if (channel_name == "class_edit" || channel_name == "loadout_showcase_entered") {
				save();
			}
			return lui_cod_lua_call_notify_server_hook.invoke<int>(lua_vm);
		}

		int online_loot_get_item_quantity_stub(void* _this, int controller_index, const std::uint32_t id) {
			return 1;
		}

		/*
		utils::hook::iat_detour create_window_ex_w_hook;
		HWND create_window_ex_w_stub(DWORD dw_ex_style, LPCWSTR lp_class_name, LPCWSTR lp_window_name, DWORD dw_style, int x, int y, int n_width, int n_height,
			HWND h_wnd_parent, HMENU h_menu, HINSTANCE h_instance, LPVOID lp_param)
		{
			LOG("Component/Inventory", DEBUG, "CWExW: {}", lp_class_name ? utils::string::convert(lp_class_name) : "<null>");
			return create_window_ex_w_hook.invoke<HWND>(dw_ex_style, lp_class_name, lp_window_name, dw_style, x, y, n_width, n_height, h_wnd_parent, h_menu,
				h_instance, lp_param);
		}

		utils::hook::iat_detour register_class_ex_w_hook;
		ATOM register_class_ex_w_stub(const WNDCLASSEXW* lp_wnd_class) {
			LOG("Component/Inventory", DEBUG, "RCExW: {}", lp_wnd_class && lp_wnd_class->lpszClassName ? utils::string::convert(lp_wnd_class->lpszClassName) : "<null>");
			return register_class_ex_w_hook.invoke<ATOM>(lp_wnd_class);
		}

		utils::hook::iat_detour register_class_w_hook;
		ATOM register_class_w_stub(const WNDCLASSW* lp_wnd_class) {
			LOG("Component/Inventory", DEBUG, "RCW: {}", lp_wnd_class && lp_wnd_class->lpszClassName ? utils::string::convert(lp_wnd_class->lpszClassName) : "<null>");
			return register_class_w_hook.invoke<ATOM>(lp_wnd_class);
		}
		*/
	}

	struct component final : generic_component {
		void post_load() override {
			auth::on_stats_init(load);
			
			lua::hook::create(lui_cod_lua_call_notify_server_hook, "Engine.NotifyServer", lui_cod_lua_call_notify_server_stub);
		}

		void find_signatures(memory::signature_store& batch) override {

			batch.add(SETUP_POINTER(DDL_LocallyObfuscate), "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 80 79 ? ? 48 8B F2 41 8B F8 48 8B D9 75");

			batch.add(SETUP_POINTER(DDL_LocallyUnObfuscate), "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 80 79 ? ? 48 8B F2 41 8B F8 48 8B D9 74");

			batch.add(SETUP_POINTER(CL_PlayerData_GetDDLBuffer), "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B E9 49 63 F8");

			if (identification::game::is_config(identification::game::Beta)) {
				batch.add(SETUP_POINTER(Online_Loot_GetItemQuantity), "48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B E9 41 8B F8");
			}
			else {
				batch.add(SETUP_POINTER(Online_Loot_GetItemQuantity), "E8 ? ? ? ? 48 8B 5C 24 ? 85 C0 0F 84", SETUP_MOD(add(1).rip()));
			}

		}

		void post_unpack() override {
			utils::hook::jump(Online_Loot_GetItemQuantity, online_loot_get_item_quantity_stub);

			command::add("inventory_load", load);
			command::add("inventory_save", save);
		}
	};
}

REGISTER_COMPONENT(inventory::component)

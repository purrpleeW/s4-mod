#include "common.hpp"
#include "component/scheduler.hpp"
#include "engine/s4/enums/CoDClanTagTypes.hpp"
#include "engine/s4/lua_State.hpp"
#include "identification/game.hpp"
#include "utils/string.hpp"
#include "clan_tag.hpp"

#include <cstdio>
#include <cstring>
#include <loader/component_loader.hpp>
#include <utils/concurrency.hpp>
#include <unordered_map>
#include <utils/hook.hpp>
#include <utils/properties.hpp>
#include <game/game.hpp>
#include <component/lua/lua_hook.hpp>
#include <component/lua/lua_vm.hpp>
#include <utils/hook.hpp>

namespace clan_tag {
	namespace {
		enum ClanTagColors {
			Red          = '1',
			Green        = '2',
			Yellow       = '3',
			Blue         = '4',
			Cyan         = '5',
			Pink         = '6',
			White        = '7',
			Team         = '8',
			Gray         = '9',
			Black        = '0',
			Crimson      = '&',
			Purple       = '*',
			LightGreen   = '(',
			LighestGreen = '.',
			PowderBlue   = ')',
			NavyBlue     = ';',
			BurntOrange  = ',',
		};

		std::array<std::string, 12> dev_clan_tags = {
			"BNOX", "DW", "HMS", "IW", "RVN", "SHG", "3ARC", "TFB", "VV", "ATVI", "BLIZ", "KING"
		};

		std::unordered_map<std::string, ClanTagColors> custom_clan_tag_colors = {
			{ "ZERO",   ClanTagColors::Blue },
			{ "Gen8",   ClanTagColors::Pink },
			{ "TARD",   ClanTagColors::Red },
			{ "PARA",   ClanTagColors::Yellow },
			{ "OF",     ClanTagColors::Cyan },
			{ "$N2",    ClanTagColors::Pink },
			{ "wlw",    ClanTagColors::Purple },
			{ "atu",    ClanTagColors::NavyBlue },
			{ "CoD",    ClanTagColors::LightGreen },
			{ "Cat", 	ClanTagColors::Purple },
			{ "VON", 	ClanTagColors::Crimson },
			{ "iw8m", 	ClanTagColors::PowderBlue },
			{ "s4m", 	ClanTagColors::Yellow },
			{ "Q3", 	ClanTagColors::Crimson },
			{ "Q4", 	ClanTagColors::LighestGreen },
			{ "Bnuy",	ClanTagColors::Pink },
			{ "Bunny",	ClanTagColors::Purple },
			{ "CDHUB",	ClanTagColors::BurntOrange },
			{ "13K",	ClanTagColors::BurntOrange },
			{ "KYS",	ClanTagColors::LightGreen },
			{ "Copy",	ClanTagColors::Yellow },
			{ "Paste",	ClanTagColors::NavyBlue },
			{ "ALDI",	ClanTagColors::Cyan },
			{ "LIDL",	ClanTagColors::Blue },
			{ "HRT",	ClanTagColors::Purple },
			{ "BKFL",	ClanTagColors::Green },
			{ "HPME",	ClanTagColors::Crimson },
			{ "STMBK",	ClanTagColors::Crimson },
			{ "PENIS",	ClanTagColors::Red },
			{ "BBC",	ClanTagColors::Black },
			{ "TYPIN",	ClanTagColors::Blue },
			{ "DNI",	ClanTagColors::Cyan },
		};

		std::unordered_map<std::string, std::pair<std::pair<std::string, ClanTagColors>, std::pair<std::string, ClanTagColors>>> split_custom_clan_tags =  {
			{ "MWII",  { { "MW", ClanTagColors::White }, { "II",  ClanTagColors::LightGreen } } },
			{ "MWIII", { { "MW", ClanTagColors::White }, { "III", ClanTagColors::Crimson } } },
		};

		utils::hook::detour cl_player_data_set_clan_tag_type_hook;
		utils::hook::detour com_player_utils_colorize_clan_tag_hook;
		utils::hook::detour com_player_utils_colorize_clan_tag_new_hook;
		utils::hook::detour lui_cod_lua_call_set_and_enable_custom_clan_tag_hook;
		utils::hook::detour lui_cod_lua_call_set_profile_data_hook;
		utils::hook::detour ui_verify_string_hook;
		utils::hook::detour ui_activision_clan_tag_allowed_for_gamer_tag_hook;

		utils::concurrency::container<std::string> current_clan_tag{};

		functions::Com_PlayerUtils_ColorizeClanTagT* Com_PlayerUtils_ColorizeClanTag{};
		functions::Com_PlayerUtils_ColorizeClanTag_NewT* Com_PlayerUtils_ColorizeClanTag_New{};
		functions::UI_VerifyStringT* UI_VerifyString{};
		functions::UI_ActivisionClanTagAllowedForGamerTagT* UI_ActivisionClanTagAllowedForGamerTag{};
		functions::CL_PlayerData_SetClanTagTypeT* CL_PlayerData_SetClanTagType{};
		functions::CL_PlayerData_SetCustomClanTagT* CL_PlayerData_SetCustomClanTag{};



		void store_clan_tag(const std::string& clan_tag) {
			utils::properties::store("playerClanTag", clan_tag);
		}

		void activate_clan_tag(std::string new_clan_tag) {
			current_clan_tag.access([&](std::string& clan_tag) {
				clan_tag = std::move(new_clan_tag);
			});
		}

		void update_clan_tag(const std::string& new_name) {
			activate_clan_tag(new_name);
			store_clan_tag(new_name);
		}

		const char* get_clan_tag() {
			const auto name = current_clan_tag.copy();
			return utils::string::va("%.*s", static_cast<int>(name.size()), name.data());
		}

		void set_clan_tag(const std::string& name) {
			update_clan_tag(name);
			CL_PlayerData_SetCustomClanTag(0, name.c_str());
		}

		void cl_player_data_set_clan_tag_type_stub(int controller_index, s4::CoDClanTagTypes clan_tag_type) {
			return cl_player_data_set_clan_tag_type_hook.invoke<void>(controller_index, s4::CoDClanTagTypes::TAG_DEVELOPER);
		}

		void com_player_utils_colorize_clan_tag_end_handler(const char* raw_tag, char* out_buffer, std::size_t out_buffer_size, const std::uint8_t post_color) {
			for (const auto& [custom_tag, tag_type] : custom_clan_tag_colors) {
				if (utils::string::to_lower(raw_tag) != utils::string::to_lower(custom_tag)) {
					continue;
				}

				snprintf(out_buffer, out_buffer_size, "^%c%s^%c", tag_type, raw_tag, post_color);
				return;
			}

			for (const auto& [custom_tag, tag_type] : split_custom_clan_tags) {
				if (utils::string::to_lower(raw_tag) != utils::string::to_lower(custom_tag)) {
					continue;
				}

				const auto [pt1_tag, pt1_clr] = tag_type.first;
				const auto [pt2_tag, pt2_clr] = tag_type.second;

				snprintf(out_buffer, out_buffer_size, "^%c%s^%c%s^%c", pt1_clr, pt1_tag.c_str(), pt2_clr, pt2_tag.c_str(), post_color);
				return;
			}

			snprintf(out_buffer, out_buffer_size, "%s^%c", raw_tag, post_color);
		}

		void com_player_utils_colorize_clan_tag_stub(const char* raw_tag, s4::CoDClanTagTypes tag_type, char* out_buffer, std::size_t out_buffer_size, const std::uint8_t post_color) {
			for (const auto& dev_tag : dev_clan_tags) {
				if (utils::string::to_lower(raw_tag) != utils::string::to_lower(dev_tag)) {
					continue;
				}
				return com_player_utils_colorize_clan_tag_hook.invoke<void>(raw_tag, tag_type, out_buffer, out_buffer_size, post_color);
			}
			com_player_utils_colorize_clan_tag_end_handler(raw_tag, out_buffer, out_buffer_size, post_color);
		}

		void com_player_utils_colorize_clan_tag_new_stub(const char* raw_tag, s4::CoDClanTagTypes tag_type, std::uint32_t a3, char* out_buffer, std::size_t out_buffer_size,
			const std::uint8_t post_color)
		{
			for (const auto& dev_tag : dev_clan_tags) {
				if (utils::string::to_lower(raw_tag) != utils::string::to_lower(dev_tag)) {
					continue;
				}
				return com_player_utils_colorize_clan_tag_hook.invoke<void>(raw_tag, tag_type, a3, out_buffer, out_buffer_size, post_color);
			}
			com_player_utils_colorize_clan_tag_end_handler(raw_tag, out_buffer, out_buffer_size, post_color);
		}

		int lui_cod_lua_call_set_and_enable_custom_clan_tag_stub(s4::lua_State* lua_vm) {
			const char* clan_tag = lua::vm::get_lstring(lua_vm, 2, 0);
			set_clan_tag(std::string(clan_tag));
			return 0;
		}

		int lui_cod_lua_call_set_profile_data_stub(s4::lua_State* lua_vm) {
			clan_tag::apply_clan_tag();
			return lui_cod_lua_call_set_profile_data_hook.invoke<int>(lua_vm);
		}

		bool ui_activision_clan_tag_allowed_for_gamer_tag_stub(int controller_index, const char* clan_tag) {
			return true;
		}

		bool ui_verify_string_stub(const char* str, const wchar_t* str_orig) {
			return true;
		}
	}

	void apply_clan_tag() {
		const auto stored_tag = utils::properties::load("playerClanTag");
		if (stored_tag) {
			activate_clan_tag(*stored_tag);
			CL_PlayerData_SetCustomClanTag(0, get_clan_tag());
		}
	}

	struct component final : client_component {
		void post_load() override {
			lua::hook::create(lui_cod_lua_call_set_and_enable_custom_clan_tag_hook,
				"Engine.SetAndEnableCustomClanTag", lui_cod_lua_call_set_and_enable_custom_clan_tag_stub);

			lua::hook::create(lui_cod_lua_call_set_profile_data_hook,
				"Engine.SetProfileData", lui_cod_lua_call_set_profile_data_stub);
		}

		void find_signatures(memory::signature_store& batch) override {
			if (identification::game::is_greater_or_eq("1.24.1")) {
				batch.add(SETUP_POINTER(Com_PlayerUtils_ColorizeClanTag_New), "E8 ? ? ? ? 43 C6 44 37 ? ? B8", SETUP_MOD(add(1).rip()));
			}
			else {
				batch.add(SETUP_POINTER(Com_PlayerUtils_ColorizeClanTag), "E8 ? ? ? ? 48 8D 54 24 ? 48 8B CF E8 ? ? ? ? 48 8B 6C 24", SETUP_MOD(add(1).rip()));
			}

			batch.add(SETUP_POINTER(CL_PlayerData_SetCustomClanTag), "E8 ? ? ? ? 4C 8B 7C 24 ? 4C 8B 74 24 ? 4C 8B 6C 24 ? 4C 8B 64 24 ? 48 8B B4 24 ? ? ? ? 48 8B AC "
				"24", SETUP_MOD(add(1).rip()));

			batch.add(SETUP_POINTER(UI_ActivisionClanTagAllowedForGamerTag), "E8 ? ? ? ? 84 C0 74 ? 33 D2 48 8B CB E8 ? ? ? ? 84 C0", SETUP_MOD(add(1).rip()));

			batch.add(SETUP_POINTER(UI_VerifyString), "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 41 54 41 55 41 56 41 57 48 83 EC ? 44 0F B6 01");

			batch.add(SETUP_POINTER(CL_PlayerData_SetClanTagType), "E8 ? ? ? ? 4C 8B C3 B2 ? 8B CF", SETUP_MOD(add(1).rip()));
		}

		void post_unpack() override {
			cl_player_data_set_clan_tag_type_hook.create(CL_PlayerData_SetClanTagType, cl_player_data_set_clan_tag_type_stub);
			ui_activision_clan_tag_allowed_for_gamer_tag_hook.create(UI_ActivisionClanTagAllowedForGamerTag,
				ui_activision_clan_tag_allowed_for_gamer_tag_stub);
			ui_verify_string_hook.create(UI_VerifyString, ui_verify_string_stub);

			if (Com_PlayerUtils_ColorizeClanTag) {
				com_player_utils_colorize_clan_tag_hook.create(Com_PlayerUtils_ColorizeClanTag, com_player_utils_colorize_clan_tag_stub);
			}
			else if (Com_PlayerUtils_ColorizeClanTag_New) {
				com_player_utils_colorize_clan_tag_new_hook.create(Com_PlayerUtils_ColorizeClanTag_New, com_player_utils_colorize_clan_tag_new_stub);
			}

			scheduler::once<scheduler::pipeline::client>([] {
				apply_clan_tag();
			});
		}
	};
}

//REGISTER_COMPONENT(clan_tag::component)

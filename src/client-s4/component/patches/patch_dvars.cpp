#include "common.hpp"
#include "game/dvars.hpp"
#include "game/game.hpp"
#include "identification/game.hpp"

#include "component/console/console.hpp"

#include <component/scheduler.hpp>
#include <loader/component_loader.hpp>
#include <utils/hook.hpp>

namespace patches::dvars {
	namespace {
		struct dvar_patch_base {
			const std::string_view original;
			const std::uint64_t hashed;
		};

		struct dvar_value_patch : public dvar_patch_base {
			const s4::DvarValue dvar_value;
			const std::uint64_t beta_hashed = 0;
		};

		struct dvar_flags_patch : public dvar_patch_base {
			const std::uint32_t /*s4::DvarFlags*/ dvar_flags;
		};

		utils::hook::detour dvar_register_bool_hook;
		utils::hook::detour dvar_register_int_hook;
		utils::hook::detour dvar_register_variant_hook;
		utils::hook::detour dvar_set_variant_hook;
		utils::hook::detour lua_shared_lua_call_get_dvar_bool_hook;

		constexpr dvar_flags_patch dvar_patched_flags[] = {
			// nothing.
		};

		constexpr dvar_value_patch dvar_patched_values[] = {
			{ "com_force_premium", 			0x1079AB2470343ED3, s4::DvarValue::create(true), 0x17F71C17431D00CB }, // the beta obviously is a bitch and handles hashing differently to ship
			{ "com_lan_lobby_enabled",		0x58D72B69706D55D8, s4::DvarValue::create(true), 0xFEAA8F48ABAD7868 }, // (applies to lua and dvars)
			{ "con_bindableGrave", 			0x7BB7662906168823, s4::DvarValue::create(false) },
			{ "enable_cod_account", 		0xA0B1A12E7E8F571E, s4::DvarValue::create(false), 0x9B66CE3665F82CF6 },
			{ "force_offline_enabled",		0xA85E94BD158C7CD1, s4::DvarValue::create(false), 0x0EF362245AB89019 },
			{ "force_offline_menus",		0x7622A6240A82EFEC, s4::DvarValue::create(false), 0x1FDA75DB98137B24 },
			{ "lui_dev_features_enabled",	0x56F8D5B213544072, s4::DvarValue::create(true), 0x6B9578D0A7E596BA }, 
			{ "online_lan_cross_play",		0x855914B2876D1B27,	s4::DvarValue::create(true) },
			{ "xp_dec_dc",					0xB403CABB1673EEB5, s4::DvarValue::create(false) },
			{ "ui_onlineRequired",			0x3AEECEE18BE07D15, s4::DvarValue::create(false), 0x479932F8F74C36CD },

			{ "GlobalVisibilityVar",		0xD5AD519F8233DCBD, s4::DvarValue::create(std::numeric_limits<int>::max()) },
			{ "IsLANPlayAllowed",			0x8CE8B128F164B55B, s4::DvarValue::create(true) }
		};

		bool matches_value_patch(std::uint64_t hash, const dvar_value_patch& patch) {
			const auto expected = identification::game::is_config(identification::game::Beta) ? patch.beta_hashed : patch.hashed;
			return expected != 0 && hash == expected;
		}

		// this is the boolCONDITIONS.IsSeasonalEventActive  reads (event tabs, menus and loading screens)
		constexpr std::uint64_t seasonal_event_active_hash = 0x956A9ECD78A1AB51;

		std::optional<int> seasonal_event_override() {
			if (identification::game::is("1.10.0")) {
				return 2;
			}
			if (identification::game::is("1.14.0")) {
				return 3;
			}
			return std::nullopt;
		}

		bool is_seasonal_event_toggle(std::uint64_t hash) {
			static const auto frontend_seasonal_content_hash = game::hash_dvar("lui_enable_frontend_seasonal_content");
			return hash == seasonal_event_active_hash || hash == frontend_seasonal_content_hash;
		}

		std::optional<int> current_season_override() {
			if (identification::game::is("1.10.0")) {
				return 1;
			}
			if (identification::game::is("1.14.0", "1.16.0")) {
				return 2;
			}
			if (identification::game::is_greater_or_eq("1.24.1")) {
				return 5;
			}
			return std::nullopt;
		}

		s4::dvar_t* dvar_register_bool_stub(std::uint64_t hash, bool value, std::uint32_t flags, const char* description) {
			bool value_patched = value;
			for (auto& patch : dvar_patched_values) {
				if (!matches_value_patch(hash, patch)) {
					continue;
				}
				value_patched = patch.dvar_value.enabled_;
				LOG("Component/Patches/Dvars", INFO, "Patched value of Dvar '{}'.", patch.original);
				break;
			}

			if (seasonal_event_override() && is_seasonal_event_toggle(hash)) {
				value_patched = true;
				LOG("Component/Patches/Dvars", INFO, "Enabled seasonal event Dvar '#x3{:X}'.", hash);
			}

			static bool called_dvars_init = false;
			const auto fps_counter_hash = identification::game::is_config(identification::game::Beta)
				? 0x8CA07F17A2BDF91A : 0x6BE1CA2F996E74E2;
			if (!called_dvars_init && hash == fps_counter_hash) {
				called_dvars_init = true;
				scheduler::execute<scheduler::pipeline::dvars>();
			}

			return dvar_register_bool_hook.invoke<s4::dvar_t*>(hash, value_patched, flags, description);
		}

		s4::dvar_t* dvar_register_int_stub(std::uint64_t hash, int value, int min, int max, std::uint32_t flags, const char* description) {
			const auto season_override = current_season_override();
			static const auto current_season_hash = game::hash_dvar("current_season");
			if (season_override && hash == current_season_hash) {
				auto dvar = dvar_register_int_hook.invoke<s4::dvar_t*>(hash, *season_override, min, max, flags, description);
				LOG("Component/Patches/Dvars", INFO, "Patched value of Dvar 'current_season' to {}.", *season_override);
				return dvar;
			}

			const auto event_override = seasonal_event_override();
			static const auto seasonal_event_id_hash = game::hash_dvar("seasonal_event_id_num");
			if (event_override && hash == seasonal_event_id_hash) {
				auto dvar = dvar_register_int_hook.invoke<s4::dvar_t*>(hash, *event_override, min, max, flags, description);
				LOG("Component/Patches/Dvars", INFO, "Patched value of Dvar 'seasonal_event_id_num' to {}.", *event_override);
				return dvar;
			}

			int value_patched = value;
			for (auto& patch : dvar_patched_values) {
				if (!matches_value_patch(hash, patch)) {
					continue;
				}
				value_patched = patch.dvar_value.integer_;
				LOG("Component/Patches/Dvars", INFO, "Patched value of Dvar '{}'.", patch.original);
				break;
			}

			return dvar_register_int_hook.invoke<s4::dvar_t*>(hash, value_patched, min, max, flags, description);
		}

		s4::dvar_t* dvar_register_variant_stub(std::uint64_t hash, std::uint8_t type, std::uint32_t flags, s4::DvarValue* value,
			s4::DvarLimits* domain, const char* description)
		{
			std::uint32_t flags_patched = flags;
			for (auto& patch : dvar_patched_flags) {
				if (hash != patch.hashed) {
					continue;
				}
				flags_patched = patch.dvar_flags;
				LOG("Component/Patches/Dvars", INFO, "Patched flags of Dvar '{}'.", patch.original);
				break;
			}

			auto res = dvar_register_variant_hook.invoke<s4::dvar_t*>(hash, type, flags_patched, value, domain, description);
			if (!::dvars::get_dvar_info_from_hash(hash).has_value()) {
				::dvars::insert_dvar_info(hash, std::format("#x3{:X}", hash), "");

				console::game_console.register_suggestion({
					.kind = console::detail::suggestion_kind::dvar,
					.name = std::format("#x3{:X}", hash),
					.checksum = hash,
				});
			}

			return res;
		}
	}

	struct component final : generic_component {
		void post_unpack() override {
			dvar_register_bool_hook.create(game::Dvar_RegisterBool, dvar_register_bool_stub);
			dvar_register_int_hook.create(game::Dvar_RegisterInt, dvar_register_int_stub);
			dvar_register_variant_hook.create(game::Dvar_RegisterVariant, dvar_register_variant_stub);
		}
	};
}

REGISTER_COMPONENT(patches::dvars::component)

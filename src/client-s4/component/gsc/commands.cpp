#include "common.hpp"
#include "component/command.hpp"
#include "component/gsc/script_loading.hpp"
#include "component/gsc/scripting.hpp"
#include "game/game.hpp"
#include "resource.h"

#include <loader/component_loader.hpp>
#include <memory/memory.hpp>
#include <utils/nt.hpp>
#include <utils/string.hpp>

namespace gsc::commands {
	namespace {
		constexpr auto give_weapon_script = "scripts/zeroproxy/give_weapon";

		functions::GScr_ExecEntThreadT* GScr_ExecEntThread{};
		functions::Scr_AddStringT* Scr_AddString{};
		void* PlayerCmd_GiveWeapon{};
		void* PlayerCmd_GiveMaxAmmo{};
		void* PlayerCmd_SwitchToWeapon{};
		void* PlayerCmd_LoadWeaponsForPlayer{};
		void* PlayerCmd_HasLoadedViewWeapons{};
		bool weapon_methods_available{};

		memory::scanned_result<void> get_function_entry(memory::scanned_result<void> match) {
			DWORD64 image_base{};
			auto function = RtlLookupFunctionEntry(match.as<std::uintptr_t>(), &image_base, nullptr);
			for (auto depth = 0; function && depth < 16; depth++) {
				const auto* unwind = reinterpret_cast<const std::uint8_t*>(image_base + function->UnwindData);
				if (!((unwind[0] >> 3) & UNW_FLAG_CHAININFO)) {
					return memory::scanned_result<void>(image_base + function->BeginAddress);
				}

				const auto chain_offset = (4 + unwind[2] * 2 + 3) & ~3;
				function = reinterpret_cast<PRUNTIME_FUNCTION>(image_base + function->UnwindData + chain_offset);
			}

			return memory::scanned_result<void>(nullptr);
		}

		bool register_weapon_method(const std::string& name, const void* function) {
			if (!function) {
				return false;
			}

			const auto address = reinterpret_cast<std::uintptr_t>(function);
			auto library = utils::nt::library::get_by_address(function);
			std::string pattern;
			for (std::size_t i = 0; i < sizeof(address); i++) {
				pattern += std::format("{:02X} ", (address >> (i * 8)) & 0xFF);
			}

			std::uint16_t method_id{};
			for (auto reference : memory::vectored_sig_scan(library, pattern, 8)) {
				if (!library.is_address_in_accessible_range(reference.as<std::uintptr_t>() - sizeof(std::uint64_t))) {
					continue;
				}

				const auto id = reference.sub(sizeof(std::uint64_t)).as<const std::uint64_t&>();
				if (id < 0x8000 || id >= 0x9000) {
					continue;
				}

				if (method_id && method_id != id) {
					LOG("Component/GSC/Commands", WARN, "Ambiguous native method registration for '{}'", name);
					return false;
				}

				method_id = static_cast<std::uint16_t>(id);
			}

			if (!method_id) {
				LOG("Component/GSC/Commands", WARN, "Could not find the native method registration for '{}'", name);
				return false;
			}

			gsc_ctx->meth_add(name, method_id);
			LOG("Component/GSC/Commands", DEBUG, "Registered '{}' as native method 0x{:04X}", name, method_id);
			return true;
		}

		bool is_loaded_weapon(const std::string& name) {
			return game::DB_XAssetExists(s4::XAssetType::ASSET_TYPE_WEAPON, name.c_str())
				&& !game::DB_IsXAssetDefault(s4::XAssetType::ASSET_TYPE_WEAPON, name.c_str());
		}

		int find_table_row(s4::StringTable* table, int column, std::string_view value) {
			for (auto row = 0; row < table->row_count_; row++) {
				if (value == game::StringTable_GetColumnValueForRow(table, row, column)) {
					return row;
				}
			}

			return -1;
		}

		int find_table_column(s4::StringTable* table, std::string_view value) {
			for (auto column = 0; column < table->column_count_; column++) {
				if (value == game::StringTable_GetColumnValueForRow(table, 0, column)) {
					return column;
				}
			}

			return -1;
		}

		std::optional<std::string> build_weapon_name(const std::string& name) {
			s4::StringTable* stats{};
			if (!game::Stringtable_GetAssetNonDefault("mp/statstable.csv", &stats)) {
				LOG("Component/GSC/Commands", WARN, "The weapon stats table might not be loaded");
				return {};
			}

			auto stats_row = find_table_row(stats, 5, name);
			if (stats_row < 0) {
				stats_row = find_table_row(stats, 4, name.substr(0, name.find("_mp")));
			}
			if (stats_row < 0) {
				return name;
			}

			const auto defaults = utils::string::split(game::StringTable_GetColumnValueForRow(stats, stats_row, 9), ' ');
			if (defaults.empty()) {
				return name;
			}

			s4::StringTable* attachments{};
			if (!game::Stringtable_GetAssetNonDefault("mp/attachmentmap.csv", &attachments)) {
				LOG("Component/GSC/Commands", WARN, "The weapon attachment table might not be loaded");
				return {};
			}

			const std::array rows = {
				find_table_row(attachments, 0, name),
				find_table_row(attachments, 0, game::StringTable_GetColumnValueForRow(stats, stats_row, 4)),
				find_table_row(attachments, 0, game::StringTable_GetColumnValueForRow(stats, stats_row, 2))
			};

			std::vector<std::string> resolved_attachments;
			for (const auto& type : defaults) {
				if (type.empty() || type == "none") {
					continue;
				}

				auto attachment = type;
				const auto column = find_table_column(attachments, type);
				for (const auto row : rows) {
					const auto* mapped = game::StringTable_GetColumnValueForRow(attachments, row, column);
					if (*mapped) {
						attachment = mapped;
						break;
					}
				}

				if (!game::DB_XAssetExists(s4::XAssetType::ASSET_TYPE_ATTACHMENT, attachment.c_str())
					|| game::DB_IsXAssetDefault(s4::XAssetType::ASSET_TYPE_ATTACHMENT, attachment.c_str())) {
					LOG("Component/GSC/Commands", WARN, "Default attachment '{}' for '{}' is not loaded", attachment, name);
					return {};
				}

				resolved_attachments.push_back(attachment);
			}

			std::ranges::sort(resolved_attachments);
			resolved_attachments.erase(std::unique(resolved_attachments.begin(), resolved_attachments.end()), resolved_attachments.end());

			auto weapon_name = name;
			for (const auto& attachment : resolved_attachments) {
				weapon_name += "+" + attachment;
			}

			return weapon_name;
		}

		void give_weapon(const command::client_info& client, const command::params_sv& params) {
			if (params.size() != 2 || params[1].empty()) {
				LOG("Component/GSC/Commands", INFO, "Usage: scr_giveWeapon <weapon id>");
				return;
			}

			if (!command::check_running() || game::Com_FrontEnd_IsInFrontEnd()) {
				LOG("Component/GSC/Commands", INFO, "Start a match before using scr_giveWeapon");
				return;
			}

			if (!command::check_cheats(client.num_)) {
				return;
			}

			if (!scripting::should_enable_injector() || !GScr_ExecEntThread || !Scr_AddString || !weapon_methods_available) {
				LOG("Component/GSC/Commands", WARN, "scr_giveWeapon requires GSC support for this game build!");
				return;
			}

			auto weapon_name = std::string(params[1]);
			if (!is_loaded_weapon(weapon_name)) {
				const auto multiplayer_name = weapon_name + "_mp";
				if (weapon_name.ends_with("_mp") || !is_loaded_weapon(multiplayer_name)) {
					LOG("Component/GSC/Commands", WARN, "Incorrect or unloaded weapon ID '{}'. Example: scr_giveWeapon s4_ar_stango44_mp", weapon_name);
					return;
				}

				weapon_name = multiplayer_name;
			}

			const auto complete_weapon = build_weapon_name(weapon_name);
			if (!complete_weapon) {
				return;
			}

			const auto handle = get_main_handle(give_weapon_script);
			if (!handle) {
				LOG("Component/GSC/Commands", WARN, "The script for giving weapons isn't loaded.");
				return;
			}

			LOG("Component/GSC/Commands", INFO, "Giving '{}' to client {}!", weapon_name, client.num_);
			const auto context = game::ScriptContext_Server();
			Scr_AddString(context, complete_weapon->c_str());
			const auto thread = GScr_ExecEntThread(client.entity_, handle, 1);
			game::Scr_FreeThread(context, thread);
		}
	}

	struct component final : client_component {
		void find_signatures(memory::signature_store& batch) override {
			batch.add(SETUP_POINTER(GScr_ExecEntThread), "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 41 8B F8 8B F2 48 8B D9 E8 ? ? ? ? 44 0F BF 03 33 C9 48 89 4C 24 ?"
				" 45 33 C9 48 89 4C 24 ? 33 D2 48 89 4C 24 ? 48 8B C8");

			batch.add(SETUP_POINTER(Scr_AddString), "48 89 5C 24 ? 57 48 83 EC ? 48 8B FA E8 ? ? ? ? 33 D2 41 B1 ? 48 8B CF 48 8B D8 C6 40 ? 02 44 8D 42 ? E8");

			batch.add(SETUP_POINTER(PlayerCmd_GiveWeapon), "45 33 C0 48 8B D3 B9 DA 14 00 00 E8 ? ? ? ?", get_function_entry);

			batch.add(SETUP_POINTER(PlayerCmd_GiveMaxAmmo), "2B D8 85 DB 7E ? 44 0F B6 44 24 ? 48 8D 54 24 ? 44 8B CB C7 44 24 ? 00 00 00 00", get_function_entry);

			batch.add(SETUP_POINTER(PlayerCmd_SwitchToWeapon), "48 89 5C 24 ? 57 48 83 EC ? 80 7A ? ? 48 8B F9 75 ? 48 63 02 48 69 D8 ? ? ? ?"
				" 48 03 1D ? ? ? ? 48 83 BB ? ? ? ? ? 75 ? 48 83 BB ? ? ? ? ? 75 ? 48 8B D1 B9 5D 0E 00 00 E8 ? ? ? ? EB ?"
				" 48 8B D7 B9 5E 0E 00 00 E8 ? ? ? ? 33 DB 45 33 C0");

			batch.add(SETUP_POINTER(PlayerCmd_LoadWeaponsForPlayer), "B9 8E 05 00 00 E8 ? ? ? ?", get_function_entry);
			
			batch.add(SETUP_POINTER(PlayerCmd_HasLoadedViewWeapons), "B9 91 05 00 00 E8 ? ? ? ?", get_function_entry);
		}

		void post_unpack() override {
			command::add_sv("scr_giveWeapon", give_weapon);

			if (scripting::should_enable_injector()) {
				weapon_methods_available = register_weapon_method("zeroproxy_giveweapon", PlayerCmd_GiveWeapon)
					&& register_weapon_method("zeroproxy_givemaxammo", PlayerCmd_GiveMaxAmmo)
					&& register_weapon_method("zeroproxy_switchtoweapon", PlayerCmd_SwitchToWeapon)
					&& register_weapon_method("zeroproxy_loadweaponsforplayer", PlayerCmd_LoadWeaponsForPlayer)
					&& register_weapon_method("zeroproxy_hasloadedviewweapons", PlayerCmd_HasLoadedViewWeapons);
			}

			on_begin_scripts([] {
				if (weapon_methods_available) {
					register_script(std::format("{}.gsc", give_weapon_script), GSC_GIVE_WEAPON);
				}
			});
		}
	};
}

REGISTER_COMPONENT(gsc::commands::component)

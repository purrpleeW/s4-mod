#include "common.hpp"
#include "component/gsc/scripting.hpp"

#include "component/gsc/script_loading.hpp"
#include "game/game.hpp"
#include "identification/game.hpp"
#include "utils/flags.hpp"

#include <loader/component_loader.hpp>
#include <utils/hook.hpp>

namespace gsc::scripting {
	std::unordered_map<int, std::unordered_map<std::string, int>> fields_table;

	std::unordered_map<std::string, std::unordered_map<std::string, const char*>> script_function_table;
	std::unordered_map<std::string, std::vector<std::pair<std::string, const char*>>> script_function_table_sort;
	std::unordered_map<const char*, std::pair<std::string, std::string>> script_function_table_rev;

	std::string current_file;

	namespace {
		functions::G_MainMP_ShutdownGameT* G_MainMP_ShutdownGame{};
		functions::G_MainSP_ShutdownGameT* G_MainSP_ShutdownGame{};
		functions::ProcessScriptT* ProcessScript{};
		functions::Scr_AddClassFieldT* Scr_AddClassField{};
		functions::Scr_SetThreadPositionT* Scr_SetThreadPosition{};

		std::string current_script_file;
		std::uint32_t current_file_id{};

		std::vector<std::function<void(bool, bool)>> shutdown_callbacks;

		utils::hook::detour g_main_mp_shutdown_game_hook;
		utils::hook::detour g_main_sp_shutdown_game_hook;
		utils::hook::detour process_script_hook;
		utils::hook::detour scr_add_class_field_hook;
		utils::hook::detour scr_set_thread_position_hook;

		void shutdown_game_pre(const bool free_scripts) {
			if (free_scripts) {
				script_function_table_sort.clear();
				script_function_table.clear();
				script_function_table_rev.clear();
			}

			for (const auto& callback : shutdown_callbacks) {
				callback(free_scripts, false);
			}
		}

		void shutdown_game_post(const bool free_scripts) {
			for (const auto& callback : shutdown_callbacks) {
				callback(free_scripts, true);
			}
		}

		void g_main_mp_shutdown_game_stub(const int full_clear) {
			shutdown_game_pre(full_clear);
			g_main_mp_shutdown_game_hook.invoke<void>(full_clear);
			shutdown_game_post(full_clear);
		}

		void g_main_sp_shutdown_game_stub(int clear_scripts, bool clear_dvars) {
			shutdown_game_pre(clear_scripts);
			g_main_sp_shutdown_game_hook.invoke<void>(clear_scripts, clear_dvars);
			shutdown_game_post(clear_scripts);
		}

		void process_script_stub(s4::scrContext_t* context, const char* filename) {
			current_script_file = filename;

			const auto file_id = atoi(filename);
			if (file_id) {
				current_file_id = static_cast<std::uint16_t>(file_id);
			}
			else {
				current_file_id = 0;
				current_file = filename;
			}

			process_script_hook.invoke<void>(context, filename);
		}

		void add_function_sort(std::uint32_t id, const char* pos) {
			std::string filename = current_file;
			if (current_file_id) {
				filename = gsc::scripting::get_token(current_file_id);
			}

			if (!script_function_table_sort.contains(filename)) {
				const auto script = gsc::find_script(s4::XAssetType::ASSET_TYPE_SCRIPTFILE, current_script_file.data(), false);
				if (script != nullptr) {
					const auto end = &gsc::get_script_bytecode(script)[gsc::get_script_bytecode_len(script)];
					script_function_table_sort[filename].emplace_back("__end__", PTR_AS(const char*, end));
				}
			}

			const auto name = gsc::scripting::get_token(id);
			auto& itr = script_function_table_sort[filename];
			itr.insert(itr.end() - 1, { name, pos });
		}

		void add_function(const std::string& file, std::uint32_t id, const char* pos) {
			const auto name = get_token(id);
			script_function_table[file][name] = pos;
			script_function_table_rev[pos] = { file, name };
		}

		std::string decrypt_string(const std::string& str) {
			// StrCypher_DecodeEncryptedString was enforced in MW 1.46, long after Vanguard started development.
			return /* StrCypher_DecodeEncryptedString(str.c_str()) */ str;
		}

		void scr_add_class_field_stub(s4::scrContext_t* context, std::uint32_t classnum, s4::scr_string_t name, std::uint32_t canonical_string,
			std::uint32_t offset)
		{
			const auto name_str = game::SL_ConvertToString(name);

			if (fields_table[classnum].find(name_str) == fields_table[classnum].end()) {
				fields_table[classnum][name_str] = offset;
			}

			scr_add_class_field_hook.invoke<void>(context, classnum, name, canonical_string, offset);
		}

		void scr_set_thread_position_stub(s4::scrContext_t* context, std::uint32_t thread_name, const char* code_pos) {
			add_function_sort(thread_name, code_pos);

			if (current_file_id) {
				const auto name = get_token(current_file_id);
				add_function(name, thread_name, code_pos);
			}
			else {
				add_function(current_file, thread_name, code_pos);
			}

			scr_set_thread_position_hook.invoke<void>(context, thread_name, code_pos);
		}
	}

	bool should_enable_injector() {
		static bool should_disable = utils::flags::has_flag("no_gsc");
		return !should_disable;
	}

	std::string get_token(std::uint32_t id) {
		return gsc::gsc_ctx->token_name(id);
	}

	void on_shutdown(const std::function<void(bool, bool)>& callback) {
		shutdown_callbacks.push_back(callback);
	}

	struct component final : client_component {
		void find_signatures(memory::signature_store& batch) override {
			// batch.add(SETUP_POINTER(G_MainMP_ShutdownGame), "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC ? 8B 05 ? ? ? ? 0F B6 F9");
			batch.add(SETUP_POINTER(G_MainMP_ShutdownGame), "40 53 57 41 56 48 83 EC ? 44 0F B6 F1");	// the other one was ShutdownGameVM... erm. (does work tho!)

			batch.add(SETUP_POINTER(G_MainSP_ShutdownGame), "E8 ? ? ? ? 44 8B 8C 24 ? ? ? ? 48 8D 44 24", SETUP_MOD(add(1).rip()));

			batch.add(SETUP_POINTER(Scr_AddClassField), "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 54 41 56 41 57 48 83 EC ? 0F B6 C2");

			batch.add(SETUP_POINTER(Scr_SetThreadPosition), "48 89 5C 24 ? 57 48 83 EC ? 49 8B D8 48 8B F9 44 8B C2");

			batch.add(SETUP_POINTER(ProcessScript), "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F9 48 8B DA B9 ? ? ? ? 44 8D 41 ? E8 ? ? ? ?"
				" 48 8B D3 B9 ? ? ? ? 48 8B F0 E8 ? ? ? ? 85 C0 75");
		}

		void post_unpack() override {
			if (!should_enable_injector()) {
				return;
			}

			g_main_mp_shutdown_game_hook.create(G_MainMP_ShutdownGame, g_main_mp_shutdown_game_stub);
			g_main_sp_shutdown_game_hook.create(G_MainSP_ShutdownGame, g_main_sp_shutdown_game_stub);
			scr_add_class_field_hook.create(Scr_AddClassField, scr_add_class_field_stub);
			scr_set_thread_position_hook.create(Scr_SetThreadPosition, scr_set_thread_position_stub);
			process_script_hook.create(ProcessScript, process_script_stub);
		}
	};
}

REGISTER_COMPONENT(gsc::scripting::component)

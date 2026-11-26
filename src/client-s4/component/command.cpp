#include "common.hpp"
#include "component/command.hpp"
#include "component/console/console.hpp"

#include "game/game.hpp"
#include "game/dvars.hpp"

#include <component/scheduler.hpp>
#include <loader/component_loader.hpp>
#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/memory.hpp>
#include <utils/io.hpp>


namespace command {
	namespace {
		functions::Cmd_AddCommandInternalT* Cmd_AddCommandInternal{};
		functions::Cmd_AddClientCommandListT* Cmd_AddClientCommandList{};
		functions::Cmd_RemoveCommandT* Cmd_RemoveCommand{};
		functions::Cmd_RemoveClientCommandListT* Cmd_RemoveClientCommandList{};
		functions::G_CmdsMP_ClientCommandT* G_CmdsMP_ClientCommand{};
		functions::SV_CmdsMP_RequestMapRestartT* SV_CmdsMP_RequestMapRestart{};

		s4::CmdArgs* cmd_args{};
		s4::gentity_s** g_entities{};
		std::uint32_t* gclient_s_flags_Offset{};
		std::uint32_t* gentity_s_client_Offset{};
		std::uint32_t* gentity_s_Size{};
		s4::cmd_function_s** s_cmd_functions{};
		s4::CmdArgs* sv_cmd_args{};
		s4::SvClient*(*SvClient_ms_clients)[200]{};

		utils::hook::detour cmd_add_command_internal_hook;
		utils::hook::detour cmd_add_client_command_list_hook;
		utils::hook::detour cmd_remove_command_hook;
		utils::hook::detour cmd_remove_client_command_list_hook;
		utils::hook::detour g_cmds_mp_client_command_hook;
		utils::hook::detour cmd_remove_server_command_list_hook;

		void restart_map(const bool load_scripts) {
			if (!SV_CmdsMP_RequestMapRestart || !game::Com_FrontEnd_IsInFrontEnd
				|| !game::Live_GetGameParty || !game::Party_AreWeServer) {
				LOG("Component/Command", WARN, "Map restart isn't currently available for this build");
				return;
			}

			if (!check_running() || game::Com_FrontEnd_IsInFrontEnd()) {
				LOG("Component/Command", INFO, "Start a local match before using this!");
				return;
			}

			const auto* party = game::Live_GetGameParty();
			if (!party || !game::Party_AreWeServer(party)) {
				LOG("Component/Command", INFO, "Only the host can use this command!");
				return;
			}

			SV_CmdsMP_RequestMapRestart(load_scripts, false);
		}


		void register_cmd_suggestion(const char* cmd_name) {
			console::game_console.register_suggestion({
				.kind = console::detail::suggestion_kind::command,
				.name = cmd_name,
				.description = std::nullopt,
				.checksum = game::hash_dvar(cmd_name)
			});
		}

		void remove_cmd_suggestion(const char* cmd_name) {
			console::game_console.remove_suggestion(game::hash_dvar(cmd_name));
		}

		void cmd_add_command_internal_stub(const char* cmd_name, void(*function)(), s4::cmd_function_s* alloced_cmd) {
			register_cmd_suggestion(cmd_name);
			return cmd_add_command_internal_hook.invoke<void>(cmd_name, function, alloced_cmd);
		}

		void cmd_add_client_command_list_stub(s4::ClCommandInfo* command_list, unsigned int command_list_size) {
			for (auto& cmd : std::span(command_list, command_list_size)) {
				register_cmd_suggestion(cmd.name_);
			}

			return cmd_add_client_command_list_hook.invoke<void>(command_list, command_list_size);
		}

		void cmd_remove_command_stub(const char* cmd_name) {
			remove_cmd_suggestion(cmd_name);
			return cmd_remove_command_hook.invoke<void>(cmd_name);
		}

		void cmd_remove_client_command_list_stub(s4::ClCommandInfo* command_list, unsigned int command_list_size) {
			for (auto& cmd : std::span(command_list, command_list_size)) {
				remove_cmd_suggestion(cmd.name_);
			}

			return cmd_remove_client_command_list_hook.invoke<void>(command_list, command_list_size);
		}
		
		void cmd_remove_server_command_list_stub(unsigned int command_list_size, s4::SvCommandInfo* command_list) {
			for (auto& cmd : std::span(command_list, command_list_size)) {
				remove_cmd_suggestion(cmd.name_);
			}

			return cmd_remove_server_command_list_hook.invoke<void>(command_list_size, command_list);
		}
		

		void g_cmds_mp_client_command_stub(const int client_num) {
			params_sv params = {};

			const auto command = utils::string::to_lower(params.get_name().data());
			const auto& sv_handlers = detail::sv_cmd_bind.get_handlers();
			if (const auto it = sv_handlers.find(command); it != sv_handlers.end()) {
				client_info inf(client_num);
				if (inf.is_valid()) {
					it->second(inf, params);
				}
			}

			return g_cmds_mp_client_command_hook.invoke<void>(client_num);
		}
	}

	void detail::main_handler() {
		params_cl params = {};

		const auto command = utils::string::to_lower(params.get_name().data());
		const auto& cl_handlers = detail::cl_cmd_bind.get_handlers();
		if (const auto it = cl_handlers.find(command); it != cl_handlers.end()) {
			it->second(params);
		}
	}

	void detail::add_raw(const char* name, void (*callback)()) {
		return Cmd_AddCommandInternal(name, callback, utils::memory::get_allocator()->allocate<s4::cmd_function_s>());
	}

	void detail::remove_raw(const char* name) {
		return Cmd_RemoveCommand(name);
	}

	template <>
	s4::CmdArgs* detail::cmd_args_provider<detail::cl_cmd_args_provider, s4::CmdArgs>::get() noexcept {
		return cmd_args;
	};

	template <>
	s4::CmdArgs* detail::cmd_args_provider<detail::sv_cmd_args_provider, s4::CmdArgs>::get() noexcept {
		return sv_cmd_args;
	};

	template <>
	s4::gclient_s* detail::cip_clients_provider<s4::SvClient>::get_local_client(int client_num) {
		return get_client(client_num);
	}

	template <>
	s4::gentity_s* detail::cip_clients_provider<s4::SvClient>::get_local_entity(int client_num) {
		return get_entity(client_num);
	}

	template <>
	s4::SvClient* detail::cip_clients_provider<s4::SvClient>::get_server_client(int client_num) {
		return get_sv_client(client_num);
	}

	void execute(std::string command, const bool sync) {
		game::Cbuf_AddText_AutoEncrypt(command);
	}


	s4::gclient_s* get_client(int client_num) {
		if (!g_entities || !gentity_s_Size || !gentity_s_client_Offset) {
			return nullptr;
		}

		s4::gclient_s* client = CLASS_VALUE_AT(get_entity(client_num), *gentity_s_client_Offset, s4::gclient_s*);
		return client;
	}

	s4::gentity_s* get_entity(int client_num) {
		if (!g_entities || !gentity_s_Size) {
			return nullptr;
		}

		s4::gentity_s* ent = CLASS_VALUE_AT_PTR(*g_entities, *gentity_s_Size * client_num, s4::gentity_s);
		return ent;
	}

	s4::SvClient* get_sv_client(int client_num) {
		if (SvClient_ms_clients == nullptr) {
			return nullptr;
		}
		return (*SvClient_ms_clients)[client_num];
	}

	bool check_cheats(int client_num) {
		auto client = get_client(client_num);
		auto sv_client = get_sv_client(client_num);
		if (client == nullptr || sv_client == nullptr) {
			return false;
		}
		if (dvars::get_enabled(dvars::sv_cheats)) {
			return true;
		}
		sv_client->SendServerCommand(s4::svscmd_type::SV_CMD_CAN_IGNORE, "e \"GAME/CHEATSNOTENABLED\"");
		return false;
	}

	static void broadcast_server_message(const std::string& message) {
		if (!SvClient_ms_clients) {
			return;
		}

		for (auto& client : *SvClient_ms_clients) {
			if (client == nullptr) {
				continue;
			}

			client->SendServerCommand(s4::svscmd_type::SV_CMD_CAN_IGNORE, utils::string::va("c \"%s\"", message.c_str()));
		}
	}

	bool check_running() {
		return dvars::get_enabled(dvars::get_dvar_by_name("sv_running"));
	}

	struct component final : public generic_component {
		void post_unpack() override {
			scheduler::once<scheduler::pipeline::dvars>([] {
				dvars::sv_cheats = dvars::register_bool("sv_cheats", false, 0, "Enable server cheats");
			});

			console::game_console.register_command_iterator([] { // fix stolen from iw8-lazer, way simpler than using the Cmd_AddCommandInternal hook to register suggestions
				std::vector<std::string> names{};
				auto cmd = *s_cmd_functions;
				while (cmd) {
					if (cmd->name_) {
						names.push_back(cmd->name_);
					}
					cmd = cmd->next_;
				}
				return names;
			});

			cmd_add_command_internal_hook.create(Cmd_AddCommandInternal, cmd_add_command_internal_stub);
			cmd_add_client_command_list_hook.create(Cmd_AddClientCommandList, cmd_add_client_command_list_stub);
			cmd_remove_command_hook.create(Cmd_RemoveCommand, cmd_remove_command_stub);
			cmd_remove_client_command_list_hook.create(Cmd_RemoveClientCommandList, cmd_remove_client_command_list_stub);
			g_cmds_mp_client_command_hook.create(G_CmdsMP_ClientCommand, g_cmds_mp_client_command_stub);

			add_commands();
			add_sv_commands();
		}

		void find_signatures(memory::signature_store& batch) override {
			batch.add(SETUP_POINTER(Cmd_AddCommandInternal), "48 89 5C 24 ? 4C 8B 15 ? ? ? ? 48 8B DA");

			batch.add(SETUP_POINTER(Cmd_AddClientCommandList), "48 83 EC ? 85 D2 0F 84 ? ? ? ? 48 89 5C 24 ? 4C 8D 51");

			batch.add(SETUP_POINTER(Cmd_RemoveCommand), "4C 8B 05 ? ? ? ? 4C 8D 15 ? ? ? ? 4C 8B D9");

			batch.add(SETUP_POINTER(Cmd_RemoveClientCommandList), "48 83 EC ? 8D 42 ? 4C 63 D8");

			batch.add(SETUP_POINTER(G_CmdsMP_ClientCommand), "E8 ? ? ? ? 65 48 8B 04 25 ? ? ? ? 48 8B 0C 18 80 3C 29 ? 75 ? E8 ? ? ? ? 65 48 8B 0C 25", SETUP_MOD(add(1).rip()));

			batch.add(SETUP_POINTER(SV_CmdsMP_RequestMapRestart), "40 55 53 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 45 ? 0F B6 FA 0F B6 D9");

			if (identification::game::is_config(identification::game::Beta)) {
				batch.add(SETUP_POINTER(cmd_args), "48 8D 35 ? ? ? ? 48 89 BC 24 ? ? ? ? 4C 89 A4 24 ? ? ? ? 45 8B A5", SETUP_MOD(add(3).rip()));
			}
			else if (identification::game::is_less_or_eq("1.07.1")) {
				batch.add(SETUP_POINTER(cmd_args), "48 63 05 ? ? ? ? 33 C9 89 9C 85", SETUP_MOD(add(3).rip()));
			}
			else {
				batch.add(SETUP_POINTER(cmd_args), "48 63 05 ? ? ? ? 83 BC 85 ? ? ? ? ? 0F 84 ? ? ? ? 89 9C 85", SETUP_MOD(add(3).rip()));
			}

			batch.add(SETUP_POINTER(g_entities), "48 8B 1D ? ? ? ? 48 69 D0", SETUP_MOD(add(3).rip()));

			batch.add(SETUP_POINTER(gclient_s_flags_Offset), "83 A6 ? ? ? ? ? C5 FA 11 86", SETUP_MOD(add(2)));

			batch.add(SETUP_POINTER(gentity_s_client_Offset), "48 83 BB ? ? ? ? ? 75 ? 48 83 BB ? ? ? ? ? 75 ? 48 83 BB", SETUP_MOD(add(3)));

			batch.add(SETUP_POINTER(gentity_s_Size), "48 89 5C 24 ? 57 48 83 EC ? 33 D2 48 8B D9 89 51", SETUP_MOD(add(0x3f).add(3)));

			batch.add(SETUP_POINTER(s_cmd_functions), "48 89 5C 24 ? 4C 8B 15 ? ? ? ? 48 8B DA", SETUP_MOD(add(8).rip()));

			batch.add(SETUP_POINTER(sv_cmd_args), "48 63 05 ? ? ? ? 4C 8B D2", SETUP_MOD(add(3).rip()));

			batch.add(SETUP_POINTER(SvClient_ms_clients), "4C 8D 05 ? ? ? ? 4C 8B D0 ? ? 00", SETUP_MOD(add(3).rip()));
		}

		bool should_activate() override {
			return Cmd_AddCommandInternal && Cmd_AddClientCommandList && G_CmdsMP_ClientCommand
				&& Cmd_RemoveCommand && Cmd_RemoveClientCommandList && cmd_args && g_entities
				&& gclient_s_flags_Offset && gentity_s_client_Offset && gentity_s_Size
				&& s_cmd_functions && sv_cmd_args && SvClient_ms_clients
			;
		}
	private:
		static void add_commands() {
			add("map_restart", [] {
				restart_map(true);
			});

			/* removed because it was causing 3D drawing issues, todo: fix
			add("fast_restart", [] {
				restart_map(false);
			});
			*/

			add("crash", [] {
				*reinterpret_cast<int*>(1) = 0;
			});
		}

		static void add_sv_commands() {
			add_sv("noclip", [](const client_info& inf, const params_sv& params) {
				if (!check_running() || !check_cheats(inf.num_)) {
					return;
				}

				int* client_flags = CLASS_VALUE_AT_PTR(inf.client_, *gclient_s_flags_Offset, int);
				if ((*client_flags & 1) != 0) {
					*client_flags &= 0xFFFFFFFE;
					inf.sv_client_->SendServerCommand(s4::svscmd_type::SV_CMD_CAN_IGNORE, "e \"GAME/NOCLIPOFF\"");
				}
				else {
					*client_flags |= 1;
					inf.sv_client_->SendServerCommand(s4::svscmd_type::SV_CMD_CAN_IGNORE, "e \"GAME/NOCLIPON\"");
				}
			});

			add_sv("server_message", [](const client_info& inf, const params_sv& params) {
				if (!check_running() || params.size() < 2) {
					return;
				}

				std::string message;
				for (std::size_t i = 1; i < params.size(); i++) {
					if (i > 1) {
						message += " ";
					}
					message += params[i];
				}

				broadcast_server_message(message);
			});
		}
	};
}

REGISTER_COMPONENT(command::component)

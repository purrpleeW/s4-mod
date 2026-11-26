#include "common.hpp"
#include "component/lan/server_list.hpp"

#include "component/command.hpp"
#include "component/lua/lua_hook.hpp"
#include "component/lua/lua_vm.hpp"
#include "component/lan/network.hpp"
#include "component/party.hpp"
#include "component/scheduler.hpp"
#include "component/lan/lan_funcs.hpp"
#include "game/dvars.hpp"
#include "game/game.hpp"
#include "utils/string.hpp"

#include <loader/component_loader.hpp>
#include <utils/cryptography.hpp>
#include <utils/hook.hpp>

namespace server_list {
	namespace {
		struct server_info {
			int clients;
			int max_clients;
			int bots;
			int ping;
			std::string host_name;
			std::string map_name;
			std::string game_type;
			char in_game;
			s4::netadr_t address;
			bool is_private;
		};

		struct {
			s4::netadr_t address{};
			volatile bool requesting = false;
			std::map<std::string, std::pair<s4::netadr_t, int>> queued_servers{};
		} master_state;

		utils::hook::detour lui_cod_lua_call_build_server_list_hook;
		utils::hook::detour lui_cod_lua_call_create_servers_data_models_hook;
		utils::hook::detour lui_cod_lua_call_get_server_count_hook;
		utils::hook::detour lui_cod_lua_call_join_server_hook;
		utils::hook::detour lui_cod_lua_call_refresh_server_list_hook;

		std::mutex mutex;
		std::vector<server_info> servers;

		const char* ui_feeder_item_text_stub(s4::LocalClientNum_t local_client_num, int feeder_id, const int index, const int column, float* s0, float* t0,
			float* s1, float* t1, s4::Material** material);

		void refresh_server_list() {
			{
				std::lock_guard<std::mutex> _(mutex);
				servers.clear();
				master_state.queued_servers.clear();
			}

			party::reset_server_connection_state();

			if (get_master_server(master_state.address)) {
				master_state.requesting = true;
				network::send(master_state.address, "getServers", utils::string::va("S4 %i full empty", game::GetProtocolVersion()));
			}
		}

		void sort_serverlist() {
			std::stable_sort(servers.begin(), servers.end(), [](const server_info& a, const server_info& b) {
				const auto a_players = a.clients - a.bots;
				const auto b_players = b.clients - b.bots;

				if (a_players == b_players) {
					if (a.clients == b.clients) {
						return a.ping < b.ping;
					}

					return a.clients > b.clients;
				}

				return a_players > b_players;
			});
		}

		void insert_server(server_info&& server) {
			std::lock_guard<std::mutex> _(mutex);
			servers.emplace_back(std::move(server));
			sort_serverlist();
		}

		void do_frame_work() {
			auto& queue = master_state.queued_servers;
			if (queue.empty()) {
				return;
			}

			std::lock_guard<std::mutex> _(mutex);

			std::size_t queried_servers = 0;
			static const std::size_t query_limit = 3;

			for (auto i = queue.begin(); i != queue.end();) {
				if (i->second.second) {
					const auto now = game::Sys_Milliseconds();
					if (now - i->second.second > 10'000) {
						i = queue.erase(i);
						continue;
					}
				}
				else if (queried_servers++ < query_limit) {
					i->second.second = game::Sys_Milliseconds();
					network::send(i->second.first, "getInfo", i->first, ' ', s4::netadrtype_t::NA_RAW);
				}

				++i;
			}
		}

		int lui_cod_lua_call_build_server_list_stub(s4::lua_State* lua_vm) {
			// UI_RunMenuScript
			// what is this

			return 0;
		}

		int lui_cod_lua_call_create_servers_data_models_stub(s4::lua_State* lua_vm) {
			auto server_info_at_index = [](std::size_t server_index, int info) {
				return ui_feeder_item_text_stub(s4::LocalClientNum_t::LOCAL_CLIENT_0, 0, static_cast<int>(server_index), info, nullptr, nullptr, nullptr, nullptr, nullptr);
			};

			std::uint16_t global_model = game::LUI_Model_GetGlobalModel();
			std::uint16_t browser_node = game::LUI_Model_CreateModelFromPath(global_model, "frontEnd.systemLinkMenu.serverBrowser");
			std::uint16_t server_count_node = game::LUI_Model_CreateModelFromPath(browser_node, "serverCount");
			auto server_count = get_server_count();

			game::LUI_Model_SetInt(server_count_node, server_count);

			for (std::size_t idx = 0; idx < servers.size(); ++idx) {
				std::uint16_t server_node = game::LUI_Model_CreateModelFromPath(browser_node, std::to_string(idx).c_str());
				game::LUI_Model_SetString(game::LUI_Model_CreateModelFromPath(server_node, "map"), game::UI_GetMapDisplayName(server_info_at_index(idx, 1)));
				game::LUI_Model_SetString(game::LUI_Model_CreateModelFromPath(server_node, "map_ref"), server_info_at_index(idx, 3));
				game::LUI_Model_SetString(game::LUI_Model_CreateModelFromPath(server_node, "host"), server_info_at_index(idx, 2));
				game::LUI_Model_SetString(game::LUI_Model_CreateModelFromPath(server_node, "players"), server_info_at_index(idx, 4));
				game::LUI_Model_SetString(game::LUI_Model_CreateModelFromPath(server_node, "type"), server_info_at_index(idx, 5));
				game::LUI_Model_SetInt(game::LUI_Model_CreateModelFromPath(server_node, "status"), server_info_at_index(idx, 10)[0] == '0' ? 0 : 1);
				game::LUI_Model_SetString(game::LUI_Model_CreateModelFromPath(server_node, "ping"), server_info_at_index(idx, 11));
			}

			return 0;
		}

		int lui_cod_lua_call_get_server_count_stub(s4::lua_State* lua_vm) {
			lua::vm::push_integer(lua_vm, get_server_count());
			return 1;
		}

		int lui_cod_lua_call_join_server_stub(s4::lua_State* lua_vm) {
			if (lua::vm::get_stack_size(lua_vm) != 2) {
				return 0;
			}

			int server_index = lua::vm::get_integer(lua_vm, 2);

			// UI_RunMenuScript
			if (server_index >= 0 && server_index < get_server_count()) {
				servers[server_index].address.type_ = s4::netadrtype_t::NA_BROADCAST;
				party::connect(servers[server_index].address);
			}

			return 0;
		}

		int lui_cod_lua_call_refresh_server_list_stub(s4::lua_State* lua_vm) {
			// UI_RunMenuScript
			refresh_server_list();

			return 0;
		}

		const char* ui_feeder_item_text_stub(s4::LocalClientNum_t local_client_num, int feeder_id, const int index, const int column, float* s0, float* t0,
			float* s1, float* t1, s4::Material** material)
		{
			if (index < get_server_count()) {
				const auto ping = servers[index].ping ? servers[index].ping : 999;

				switch (column) {
				case 1:
				case 3:
					// Map
					return servers[index].map_name.c_str();
				case 2:
					// Party Leader
					return servers[index].host_name.c_str();
				case 4:
					// #Players
					return utils::string::va("%d (%d)", servers[index].clients, servers[index].max_clients);
				case 5:
					// Type
					return servers[index].game_type.c_str();
				case 10:
					// Status [In Game=0/In Lobby=1]
					return servers[index].in_game ? "0" : "1";
				case 11:
					// Ping
					if (ping < 75) {
						return utils::string::va("^2%d", ping);
					}
					else if (ping < 150) {
						return utils::string::va("^3%d", ping);
					}
					return utils::string::va("^1%d", ping);
				}
			}

			return "";
		}
	}

	bool get_master_server(s4::netadr_t& address) {
		const auto* master_ip = dvars::get_string(dvars::lfx_masterServerIP);
		const auto* master_port = dvars::get_string(dvars::lfx_masterServerPort);

		if (!*master_ip || !*master_port) {
			return false;
		}

		auto res = lan_funcs::NET_StringToAdr(utils::string::va("%s:%s", master_ip, master_port), &address);
		if (res) {
			address.type_ = s4::netadrtype_t::NA_RAW;
		}
		return res;
	}

	void handle_info_response(const s4::netadr_t& address, const utils::info_string& info) {
		LOG("Component/ServerList", DEBUG, "Received infoResponse from {}", address.to_string_type_ignorant());
		if (info.get("gamename") != "S4") {
			LOG("Component/ServerList", DEBUG, "infoResponse @ {}: cancelling because game was not S4", address.to_string_type_ignorant());
			return;
		}

		auto challenge = info.get("challenge");
		LOG("Component/ServerList", DEBUG, "infoResponse @ {}: challenge='{}'", address.to_string_type_ignorant(), challenge);

		int start_time{};
		const auto now = game::Sys_Milliseconds();

		{
			std::lock_guard<std::mutex> _(mutex);

			auto entry = master_state.queued_servers.find(challenge);
			if (entry == master_state.queued_servers.end() || !entry->second.second) {
				LOG("Component/ServerList", DEBUG, "infoResponse @ {}: could not find challenge in queue, cancelling", address.to_string_type_ignorant());
				return;
			}

			LOG("Component/ServerList", DEBUG, "infoResponse @ {}: found challenge in queue, erasing", address.to_string_type_ignorant());
			start_time = entry->second.second;
			master_state.queued_servers.erase(entry);
		}

		server_info server{};
		server.address = address;
		server.host_name = info.get("hostname");
		server.map_name = info.get("mapname");
		server.game_type = info.get("gametype");
		server.clients = atoi(info.get("clients").data());
		server.max_clients = atoi(info.get("sv_maxclients").data());
		server.bots = atoi(info.get("bots").data());
		server.ping = std::min(now - start_time, 999);
		//server.is_private = atoi(info.get("isPrivate").data()) == 1;

		server.in_game = atoi(info.get("inlobby").data()) == 0;

		auto party = game::Live_GetGameParty();
		if (party != nullptr && strcmp(party::session_info_to_bytes(party->session_->dyn_.session_info_).data(), info.get("sessioninfo").data()) == 0) {
			// ignore our own session.
			return;
		}

		if (info.get("protocol") != utils::string::va("%i", game::GetProtocolVersion())) {
			return;
		}

		LOG("Component/ServerList", DEBUG, "infoResponse @ {}: inserting server", address.to_string_type_ignorant());
		insert_server(std::move(server));
	}

	int get_player_count() {
		std::lock_guard<std::mutex> _(mutex);
		auto count = 0;
		for (const auto& server : servers) {
			count += server.clients - server.bots;
		}
		return count;
	}

	int get_server_count() {
		std::lock_guard<std::mutex> _(mutex);
		return static_cast<int>(servers.size());
	}

	struct component final : generic_component {
		void post_load() override {
			scheduler::once<scheduler::pipeline::dvars>([]() {
				// add dvars to change destination master server ip/port
				dvars::lfx_masterServerIP = dvars::register_string("lfx_masterServerIP", "26.194.186.59", 0 /* DVAR_FLAG_NONE */,
					"IP of the destination master server to connect to");
				dvars::lfx_masterServerPort = dvars::register_string("lfx_masterServerPort", "20810", 0 /* DVAR_FLAG_NONE */,
					"Port of the destination master server to connect to");
			});

			lua::hook::create(lui_cod_lua_call_build_server_list_hook, "Lobby.BuildServerList", lui_cod_lua_call_build_server_list_stub);
			lua::hook::create(lui_cod_lua_call_create_servers_data_models_hook, "Lobby.CreateServersDataModels", lui_cod_lua_call_create_servers_data_models_stub);
			lua::hook::create(lui_cod_lua_call_get_server_count_hook, "Lobby.GetServerCount", lui_cod_lua_call_get_server_count_stub);
			lua::hook::create(lui_cod_lua_call_join_server_hook, "Lobby.JoinServer", lui_cod_lua_call_join_server_stub);
			lua::hook::create(lui_cod_lua_call_refresh_server_list_hook, "Lobby.RefreshServerList", lui_cod_lua_call_refresh_server_list_stub);
		}

		void post_unpack() override {
			if (game::is_server()) {
				return;
			}

			scheduler::loop<scheduler::pipeline::client>(do_frame_work);

			utils::hook::jump(game::UI_FeederItemText, ui_feeder_item_text_stub);

			network::on("serversResponse", [](const s4::netadr_t& target, const std::string_view& data, const int) {{
				std::lock_guard<std::mutex> _(mutex);
				if (!master_state.requesting || !master_state.address.matches_type_ignorant(target)) {
					return;
				}

				master_state.requesting = false;

				std::optional<std::size_t> start{};
				for (std::size_t i = 0; i + 6 < data.size(); ++i) {
					if (data[i + 6] == '\\') {
						start.emplace(i);
						break;
					}
				}

				if (!start.has_value()) {
					return;
				}

				for (auto i = start.value(); i + 6 < data.size(); i += 7) {
					if (data[i + 6] != '\\') {
						break;
					}

					s4::netadr_t address{};
					address.type_ = s4::netadrtype_t::NA_BROADCAST;
					address.local_net_id_ = s4::netsrc_t::NS_CLIENT1;
					std::memcpy(&address.ip_[0], data.data() + i + 0, sizeof(address.ip_));
					std::memcpy(&address.port_, data.data() + i + 4, sizeof(address.port_));

					std::string rnd_token{};
					LOG("Component/ServerList", DEBUG, "serversResponse: generating random token...");
					do {
						rnd_token = utils::cryptography::random::get_challenge();
					} while (master_state.queued_servers.find(rnd_token) != master_state.queued_servers.end());

					LOG("Component/ServerList", DEBUG, "serversResponse: generated random token, queuing challenge '{}' for {}", rnd_token,
						address.to_string_type_ignorant());
					master_state.queued_servers[rnd_token] = std::make_pair(address, 0);
				}
			}});

			network::on("error", [](const s4::netadr_t& target, const std::string_view& data, const int) {
				if (!master_state.address.matches_type_ignorant(target)) {
					return;
				}

				auto splits = utils::string::split(std::string(data.data(), data.size()), ' ');
				if (splits.size() <= 1) {
					LOG("Component/ServerList", WARN, "Error message was formatted incorrectly from server!");
					return;
				}

				std::vector<std::string> message_parts{};
				for (auto x = 1; x < splits.size(); x++) {
					message_parts.push_back(splits.at(x));
				}
				auto message = utils::string::join(message_parts, " ");

				network::should_report_error = true;
				network::last_error_code = splits[0];
				network::last_error_message = message;

				scheduler::once<scheduler::pipeline::client>([] {
					game::Com_SetErrorMessage(network::last_error_message.c_str());
				});
			});

			command::add("server_list_testRefreshServerList", [] {
				refresh_server_list();
			});
		}
	};
}

REGISTER_COMPONENT(server_list::component)

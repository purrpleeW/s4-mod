#include "common.hpp"
#include "component/party.hpp"

#include "component/auth.hpp"
#include "component/command.hpp"
#include "component/lan/network.hpp"
#include "component/scheduler.hpp"
#include "component/lan/server_list.hpp"
#include "component/lan/lan_funcs.hpp"
#include "engine/s4/enums/PartyJoinState.hpp"
#include "game/dvars.hpp"
#include "game/game.hpp"
#include "utils/string.hpp"

#include <loader/component_loader.hpp>
#include <string.h>
#include <utils/cryptography.hpp>
#include <utils/hook.hpp>
#include <utils/info_string.hpp>

namespace party {
	namespace {
		utils::hook::detour net_create_session_hook;
		utils::hook::detour party_atomic_host_send_join_failed_response_hook;

		connection_state server_connection_state{};
		std::string server_command{};
		std::string stored_challenge{};

		void join_session(s4::XSESSION_INFO& session_info) {
			game::Cbuf_AddText(std::format("#x3{:X} 0;#x3{:X} 1;#x3{:X} 0;#x3{:X} 0",
				game::hash_dvar("onlinegame"),
				game::hash_dvar("systemlink"),
				game::hash_dvar("xblive_privatematch"),
				game::hash_dvar("ui_opensummary")));

			// PROTECT_START;
			// works fine but will disband (most of the time?) and doesn't join online games
			game::PartyAtomic_AcceptInviteMP(0, &session_info, 0, FALSE, FALSE);

			// will not disband but requires more signatures - for some reason also doesn't allow joining on system link anymore lol??
			/*
			bool setup_status = game::PartyAtomic_SetupPotentialHostForJoining(0, &session_info, iw8::PartyType::GAME_LOBBY_ID, false, game::g_partyJoinInfo);
			iw8::PartyData* active_party = game::Party_GetActiveParty();
			if (game::Party_InParty(active_party)) {
				game::PartyAtomic_FillSubPartyInfoForParty(game::g_partyJoinInfo, active_party);
			}
			else {
				game::PartyAtomic_FillSubPartyInfoForLocalClients(game::g_partyJoinInfo);
			}

			int retry_count = game::Dvar_GetIntSafe("OLPNMRSNLM");	// pt_searchConnectAttempts
			game::PartyAtomic_StartJoin(game::g_partyJoinInfo, active_party, iw8::PartyJoinType::PJT_INVITE, iw8::PartyJoinState::PARTYJOIN_REQUEST, retry_count);

			if (!setup_status) {
				LOG("Component/Party", WARN, "Failed to setup potential host for joining.");
			}
			*/

			// does not fix online cg, infact doesn't work at all
			/*
			if (game::PartyAtomic_SetupPotentialHostForJoining(0, &session_info, s4::PartyType::GAME_LOBBY_ID, false, game::g_partyJoinInfo)) {
				game::PartyAtomic_StartJoin(game::g_partyJoinInfo, game::g_partyData, s4::PartyJoinType::PJT_INVITE, s4::PartyJoinState::PARTYJOIN_REQUEST, 6);
			}
			*/
			// PROTECT_END;
		}

		void net_create_session_stub(s4::XSESSION_INFO* out_session_info) {
			net_create_session_hook.invoke<void>(out_session_info);

			auto party = game::Live_GetGameParty();
			if (party != nullptr && game::Party_AreWeServer(party)) {
				send_heartbeat();
			}

			s4::netadr_t addr;
			if (server_list::get_master_server(addr)) {
				network::send(addr, "getBots");
			}
		}

		void party_atomic_host_send_join_failed_response_stub(s4::PartyData* party, s4::netadr_t* addr, std::uint32_t response, std::uint64_t* client_xuid) {
			LOG("Component/Party", DEBUG, "PartyAtomicHost_SendJoinFailedResponse: sending code {} to {} from {}", response,
				addr->to_string_type_ignorant(), and_rel(PTR_AS(std::uintptr_t, _ReturnAddress())));
			party_atomic_host_send_join_failed_response_hook.invoke<void>(party, addr, response, client_xuid);
		}

		bool party_host_party_restricts_f2p_users_stub(const s4::PartyData* party) {
			return false;
		}
	}

	std::string session_info_to_bytes(s4::XSESSION_INFO& session_info) {
		std::string session_info_bytes{};
		for (auto x = 0; x < sizeof(s4::XSESSION_INFO); x++) {
			session_info_bytes += std::format("{:02X}", PTR_AS(std::uint8_t*, &session_info)[x]);
		}
		return session_info_bytes;
	}

	bool session_info_from_bytes(s4::XSESSION_INFO* session_info, std::string bytes) {
		if (bytes.size() != sizeof(s4::XSESSION_INFO) * 2) {
			return false;
		}

		ZeroMemory(session_info, sizeof(s4::XSESSION_INFO));
		for (auto x = 0; x < sizeof(s4::XSESSION_INFO); x++) {
			std::string byte_str(&bytes[x * 2], 2);
			PTR_AS(std::uint8_t*, session_info)[x] = static_cast<std::uint8_t>(std::stoul(byte_str, nullptr, 16));
		}

		return true;
	}

	void reset_server_connection_state() {
		server_connection_state = {};
	}

	void connect(const s4::netadr_t& address) {
		server_connection_state.host = address;
		server_connection_state.challenge = utils::cryptography::random::get_challenge();

		network::send(address, "getInfo", server_connection_state.challenge, ' ', s4::netadrtype_t::NA_RAW);
	}

	void send_heartbeat() {
		s4::netadr_t target{};
		if (server_list::get_master_server(target)) {
			network::send(target, "heartbeat", std::format("S4 {}", stored_challenge));
		}
	}

	struct component final : generic_component {
		void post_unpack() override {
			stored_challenge = utils::cryptography::random::get_challenge();

			if (identification::game::is("1.10.0")) {
				party_atomic_host_send_join_failed_response_hook.create(0x253A5A0_b, party_atomic_host_send_join_failed_response_stub);
			}

			utils::hook::jump(game::PartyHost_PartyRestrictsF2PUsers, party_host_party_restricts_f2p_users_stub);

			net_create_session_hook.create(game::NET_CreateSession, net_create_session_stub);
			
			command::add("heartbeat", send_heartbeat);
			
			command::add("connect", [](const command::params_cl& params) {
				if (params.size() != 2) {
					LOG("Component/Party", INFO, "usage: connect <ip>");
					return;
				}

				s4::netadr_t target{};
				if (lan_funcs::NET_StringToAdr(params[1].data(), &target)) {
					connect(target);
				}
			});

			command::add("party_getSessionInfo", [] {
				{
					s4::XSESSION_INFO session_info = game::g_partyData->session_->dyn_.session_info_;

					std::string lobby_data_bytes{};
					for (auto x = 0; x < sizeof(session_info); x++) {
						lobby_data_bytes += std::format("{:02X}", PTR_AS(std::uint8_t*, &session_info)[x]);
					}

					LOG("Component/Party", INFO, "Party data: {}", lobby_data_bytes);
				}
				{
					s4::XSESSION_INFO session_info = game::Live_GetGameParty()->session_->dyn_.session_info_;

					std::string lobby_data_bytes{};
					for (auto x = 0; x < sizeof(session_info); x++) {
						lobby_data_bytes += std::format("{:02X}", PTR_AS(std::uint8_t*, &session_info)[x]);
					}

					LOG("Component/Party", INFO, "Lobby data: {}", lobby_data_bytes);
				}
			});

			command::add("party_joinSession", [](const command::params_cl& params) {
				if (params.size() != 2) {
					LOG("Component/Party", INFO, "usage: party_joinSession <data>");
					return;
				}

				if (params[1].size() != sizeof(s4::XSESSION_INFO) * 2) {
					LOG("Component/Party", INFO, "Exported lobby data size mismatch [expected {}, got {}]", sizeof(s4::XSESSION_INFO) * 2, params[1].size());
					return;
				}

				s4::XSESSION_INFO session_info;
				ZeroMemory(&session_info, sizeof(session_info));

				for (auto x = 0; x < sizeof(session_info); x++) {
					std::string byte_str(params[1].substr(x * 2, 2));
					PTR_AS(std::uint8_t*, &session_info)[x] = static_cast<std::uint8_t>(std::stoul(byte_str, nullptr, 16));
				}
				
				game::PartyAtomic_AcceptInviteMP(0, &session_info, 0, FALSE, FALSE);
			});
			
			network::on("getInfo", [](const s4::netadr_t& target, const std::string_view& data, const int) {
				utils::info_string info{};

				auto party = game::Live_GetGameParty();
				if (party == nullptr || !game::Party_AreWeServer(party)) {
					LOG("Component/ServerList", DEBUG, "getInfo: cancelling because no party or not server");
					return;
				}

				/*
				auto active_party = game::Party_GetActiveParty();
				if (active_party == nullptr || !game::Party_AreWeServer(active_party)) {
					LOG("Component/ServerList", DEBUG, "getInfo: cancelling because no active party or not active server");
					return;
				}
				*/

				LOG("Component/ServerList", DEBUG, "getInfo: sending infoResponse to {}", target.to_string_type_ignorant());
				// int clients_count = game::Party_CountAllMembersEvenIfInactive(party);
				int clients_max = game::Dvar_GetIntSafe(game::hash_dvar("party_maxplayers"));

				auto user_data = auth::get_local_user_data();

				info.set("challenge", std::string{ data });
				info.set("gamename", "S4");
				info.set("hostname", user_data ? user_data->gamertag_ : "S4 Host");
				info.set("mapname", game::Party_GetMapName());
				info.set("gametype", game::Party_GetGametype());
				info.set("clients", utils::string::va("%i", 1 /* clients_count */));
				info.set("sv_maxclients", utils::string::va("%i", clients_max));
				info.set("bots", "0");
				info.set("inlobby", utils::string::va("%i", game::Party_IsRunning(party)));
				info.set("protocol", utils::string::va("%i", game::GetProtocolVersion()));
				info.set("sessioninfo", session_info_to_bytes(party->session_->dyn_.session_info_));

				auto systemlink = game::Live_IsInSystemlinkLobby();
				info.set("d_systemlink", systemlink ? "1" : "0");

				network::send(target, "infoResponse", info.build(), '\n', s4::netadrtype_t::NA_RAW);
			});

			network::on("infoResponse", [](const s4::netadr_t& target, const std::string_view& data, const int) {
				const utils::info_string info{ data };
				server_list::handle_info_response(target, info);

				if (!target.matches_type_ignorant(server_connection_state.host)) {
					return;
				}

				if (!session_info_from_bytes(&server_connection_state.session_info, info.get("sessioninfo"))) {
					return;
				}

				scheduler::once<scheduler::pipeline::client>([] {
					join_session(server_connection_state.session_info);
					ZeroMemory(&server_connection_state.host, sizeof(server_connection_state.host));
				});
			});
		}
	};
}

REGISTER_COMPONENT(party::component)
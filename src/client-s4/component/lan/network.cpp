#include "common.hpp"
#include "component/lan/network.hpp"

#include "component/assets.hpp"
#include "component/command.hpp"
#include "component/scheduler.hpp"
#include "component/lan/lan_funcs.hpp"
#include "engine/s4/msg_t.hpp"
#include "engine/s4/netadr_t.hpp"
#include "game/dvars.hpp"
#include "game/game.hpp"

#include <cstdlib>
#include <loader/component_loader.hpp>
#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/storage/storage_generic.hpp>

namespace network {
	namespace {
		utils::hook::detour cl_main_mp_dispatch_connectionless_packet_hook;

		std::vector<std::uint32_t> local_interface_addresses;
		std::jthread query_thread;

		void assign_local_interface_addresses() {
			if (!local_interface_addresses.empty()) {
				return;
			}

			WSADATA wsa_data;
			if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
				LOG("Component/Network", ERROR, "[ALIA] WSAStartup failed.");
				return;
			}

			ULONG out_buf_len = 15000;
			IP_ADAPTER_ADDRESSES* adapter_addresses = PTR_AS(IP_ADAPTER_ADDRESSES*, malloc(out_buf_len));
			if (!adapter_addresses) {
				LOG("Component/Network", ERROR, "[ALIA] Memory allocation failed.");
				return;
			}

			DWORD ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, adapter_addresses, &out_buf_len);
			if (ret == ERROR_BUFFER_OVERFLOW) {
				free(adapter_addresses);
				adapter_addresses = PTR_AS(IP_ADAPTER_ADDRESSES*, malloc(out_buf_len));
				if (!adapter_addresses) {
					LOG("Component/Network", ERROR, "[ALIA] Memory allocation failed.");
					return;
				}
				ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, nullptr, adapter_addresses, &out_buf_len);
			}

			if (ret == NO_ERROR) {
				for (auto* adapter = adapter_addresses; adapter != nullptr; adapter = adapter->Next) {
					for (auto* addr = adapter->FirstUnicastAddress; addr != nullptr; addr = addr->Next) {
						SOCKADDR* sockaddr_ptr = addr->Address.lpSockaddr;
						if (sockaddr_ptr->sa_family != AF_INET) {
							continue;
						}
						local_interface_addresses.push_back(PTR_AS(sockaddr_in*, sockaddr_ptr)->sin_addr.S_un.S_addr);
					}
				}
			}
			else {
				LOG("Component/Network", ERROR, "[ALIA] GetAdaptersAddresses failed with error: {}", ret);
			}

			free(adapter_addresses);
		}

		bool is_address_present_in_local_interfaces(std::uint32_t address) {
			for (auto& a : local_interface_addresses) {
				if (a != address) {
					continue;
				}
				return true;
			}
			return false;
		}

		std::unordered_map<std::string, callback>& get_callbacks() {
			static std::unordered_map<std::string, callback> callbacks{};
			return callbacks;
		}

		ULONG string_to_sockaddr(const char* s, sockaddr* s_adr) {
			std::memset(s_adr, 0, sizeof(sockaddr));
			s_adr->sa_family = AF_INET;

			if (std::isdigit(static_cast<std::uint8_t>(s[0]))) {
				in_addr addr{};
				addr.s_addr = inet_addr(s);
				if (addr.s_addr != INADDR_NONE) {
					std::memcpy(&s_adr->sa_data[2], &addr, sizeof(in_addr));
					return 1;
				}
				return 0;
			}

			hostent* he = gethostbyname(s);
			if (!he || !he->h_addr_list || !he->h_addr_list[0]) {
				return 0;
			}

			std::vector<char*> addresses;
			for (char** p = he->h_addr_list; *p; ++p) {
				addresses.push_back(*p);
			}

			if (addresses.empty()) {
				return 0;
			}

			static std::mt19937 rng(static_cast<std::uint32_t>(std::time(nullptr)));
			std::uniform_int_distribution<int> dist(0, static_cast<int>(addresses.size() - 1));
			int idx = dist(rng);

			std::memcpy(&s_adr->sa_data[2], addresses[idx], sizeof(in_addr));

			return 1;
		}
		
		void handle_command(s4::netadr_t* address, const char* command, s4::msg_t* message, int time) {
			const auto cmd_string = utils::string::to_lower(command);
			auto& callbacks = get_callbacks();
			const auto handler = callbacks.find(cmd_string);
			const auto offset = cmd_string.size() + 5;

			if (message->cur_size_ < 0 || static_cast<std::size_t>(message->cur_size_) < offset || handler == callbacks.end()) {
				return;
			}

			const std::basic_string_view data(PTR_AS(char*, message->data_) + offset, message->cur_size_ - offset);
			try {
				handler->second(*address, data, time);
			}
			catch (const std::exception& e) {
				LOG("Component/Network", WARN, "Error \"{}\"", e.what());
			}
		}

		/*
		bool handle_command(s4::netadr_t* address, const char* command, s4::msg_t* message, int time) {
			const auto cmd_string = utils::string::to_lower(command);
			auto& callbacks = get_callbacks();
			const auto handler = callbacks.find(cmd_string);
			const auto offset = cmd_string.size() + 5;

			if (message->cur_size_ < 0 || static_cast<std::size_t>(message->cur_size_) < offset || handler == callbacks.end()) {
				return false;
			}

			const std::basic_string_view data(PTR_AS(char*, message->data_) + offset, message->cur_size_ - offset);
			try {
				handler->second(*address, data, time);
			}
			catch (const std::exception& e) {
				LOG("Component/Network", WARN, "Error \"{}\"", e.what());
			}

			return true;
		}
		*/

		// make a different way to handle this since it doesn't go through Sys_GetPacket
		/*
		bool cl_main_mp_dispatch_connectionless_packet_stub(s4::LocalClientNum_t local_client_num, s4::netadr_t* from, s4::msg_t* msg, int time) {
			if (handle_command(from, game::Cmd_Argv(0), msg, time)) {
				return true;
			}
			return cl_main_mp_dispatch_connectionless_packet_hook.invoke<bool>(local_client_num, from, msg, time);
		}
		*/

		int net_compare_base_address(const s4::netadr_t* a, const s4::netadr_t* b) {
			if (a->type_ == b->type_) {
				switch (a->type_) {
				case s4::netadrtype_t::NA_LOOPBACK:
					return a->port_ == b->port_;
				case s4::netadrtype_t::NA_IP:
				case s4::netadrtype_t::NA_RAW:
					return memcmp(a->ip_, b->ip_, 4) == 0;
				case s4::netadrtype_t::NA_BROADCAST:
					return true;
				default:
					break;
				}
			}

			return false;
		}

		int net_compare_address(const s4::netadr_t* a, const s4::netadr_t* b) {
			return net_compare_base_address(a, b) != 0 && (a->port_ == b->port_);
		}

		int get_protocol_version_stub() {
			return identification::game::get_target_game().game_version.at(3);
		}

		// todo: create custom thread for this
		int sys_get_packet_stub(s4::netadr_t* net_from, s4::msg_t* net_message) {
			int ret = 0;
			ZeroMemory(net_from, sizeof(s4::netadr_t));

			if (query_socket != INVALID_SOCKET) {
				ret = lan_funcs::Sys_RecvFrom(query_socket, net_from, net_message);

				// Other clients on this PC share our IP, but use a different query port.
				if (ret > 0 && net_from->port_ == query_socket_port
					&& (net_from->ip_[0] == 127 || is_address_present_in_local_interfaces(DEREF_PTR_AS(std::uint32_t, net_from->ip_)))) {
					ret = 0;
				}

				// woohoo
				if (ret > 0) {
					net_from->flags_ |= 1;
					net_from->type_ = s4::netadrtype_t::NA_BROADCAST;
					net_from->addr_handle_index_ = -1;
				}

				// checksum.
				if (ret > 3) {
					ret -= 3;
				}
			}

			net_message->cur_size_ = ret;
			return ret;
		}

		void receive_packets(std::stop_token stop) {
			std::array<std::uint8_t, 0x10000> packet_buffer{};

			while (!stop.stop_requested()) {
				s4::msg_t message{};
				s4::netadr_t address{};
				message.data_ = packet_buffer.data();
				message.max_size_ = static_cast<int>(packet_buffer.size());

				if (sys_get_packet_stub(&address, &message) <= 0) {
					Sleep(1);
					continue;
				}

				if (message.cur_size_ <= 4 || std::memcmp(message.data_, "\xFF\xFF\xFF\xFF", 4) != 0) {
					continue;
				}

				const std::string_view payload(PTR_AS(const char*, message.data_) + 4, message.cur_size_ - 4);
				const auto separator = payload.find_first_of(" \n");
				if (separator == std::string_view::npos || separator == 0) {
					continue;
				}

				const std::string command(payload.substr(0, separator));
				handle_command(&address, command.c_str(), &message, game::Sys_Milliseconds());
			}
		}
		/*
		int sys_get_packet_stub(s4::netadr_t* net_from, s4::msg_t* net_message, s4::NetPingInfo* net_info) {
			int ret = 0;
			ZeroMemory(net_from, sizeof(s4::netadr_t));

			if (query_socket != INVALID_SOCKET) {
				ret = lan_funcs::Sys_RecvFrom(query_socket, net_from, net_message);
				if (is_address_present_in_local_interfaces(DEREF_PTR_AS(std::uint32_t, net_from->ip_))) {
					ret = 0;
				}
				if (ret > 0) {
					net_from->flags_ |= 1;
					net_from->type_ = s4::netadrtype_t::NA_BROADCAST;
					net_from->addr_handle_index_ = -1;
				}
			}

			if (ret <= 0) {
				ret = game::NET_RecvFrom(net_message->data_, net_message->max_size_, net_from, net_info);

				if (ret == -2 || ret == -9) {
					return 0;
				}

				if (ret < 0) {
					LOG("Component/Network", WARN, "Sys_GetPacket: NET_RecvFrom failed with {} (ret was {})", lan_funcs::NET_ErrorString(), ret);
					return 0;
				}
			}

			net_message->read_count_ = 0;

			if (ret == net_message->max_size_) {
				return 0;
			}

			if (ret == 0) {
				LOG("Component/Network", WARN, "Empty or discarded packet from {}", game::NET_AdrToString(*net_from));
				return 0;
			}

			// function required for packet size offsetting
			game::Sys_VerifyCheckSum(net_message->data_, &ret);

			// this one doesn't allow things that aren't checksummed (i don't care)
			//if (!game::Sys_VerifyCheckSum(net_message->data_, &ret)) {
			//	return 0;
			//}

			net_message->cur_size_ = ret;
			net_message->target_local_net_id_ = static_cast<s4::netsrc_t>(net_message->data_[ret + 2] >> 4);
			net_from->local_net_id_ = static_cast<s4::netsrc_t>(net_message->data_[ret + 2] & 0xF);

			return 1;
		}

		int sys_send_packet_stub(iw8::netsrc_t sock, int length, const void* data, const s4::netadr_t& to, std::uint32_t flags, iw8::NetPingInfo* info) {
			std::uint8_t buffer[1264];
			std::uint32_t checksum = 0;
			int send_length = 0;
			int ret = 0;

			if (length > static_cast<int>(sizeof(buffer) - 9)) {
				LOG("Component/Network", WARN, "Sys_SendPacket: length exceeds buffer ({} > {})", length, sizeof(buffer));
				return -1;
			}

			const std::uint8_t* src = static_cast<const std::uint8_t*>(data);
			std::uint8_t* dst = buffer;
			int remaining = length;

			while (remaining > 1) {
				checksum += (src[0] << 8) | src[1];
				*dst++ = *src++;
				*dst++ = *src++;
				remaining -= 2;
			}

			if (remaining > 0) {
				checksum += *src;
				*dst++ = *src;
			}

			while (checksum >> 16) {
				checksum = (checksum & 0xFFFF) + (checksum >> 16);
			}

			std::uint16_t final_checksum = static_cast<std::uint16_t>(~checksum);

			buffer[length] = static_cast<std::uint8_t>((final_checksum >> 8) & 0xFF);
			buffer[length + 1] = static_cast<std::uint8_t>(final_checksum & 0xFF);
			buffer[length + 2] = static_cast<std::uint8_t>(ENUM_UNDER(sock) | (ENUM_UNDER(to.local_net_id_) << 4));
			send_length = length + 3;

			switch (to.type_) {
			case s4::netadrtype_t::NA_LOOPBACK:
			case s4::netadrtype_t::NA_IP:
				ret = game::NET_SendTo(buffer, send_length, &to, flags, info);
				break;
			case s4::netadrtype_t::NA_BROADCAST:
			case s4::netadrtype_t::NA_RAW:
				if (query_socket == INVALID_SOCKET) {
					ret = -9;
					break;
				}

				if (is_address_present_in_local_interfaces(DEREF_PTR_AS(const std::uint32_t, to.ip_))) {
					ret = 0;
				}
				else {
					ret = lan_funcs::Sys_SendTo(query_socket, PTR_AS(const char*, buffer), send_length, to);
				}
				break;

			default:
				LOG("Component/Network", WARN, "Sys_SendPacket: unknown address type {}", static_cast<int>(to.type_));
				return -7;
			}

			if (ret < 0) {
				LOG("Component/Network", WARN, "[NET] Sys_SendPacket failed to {}: error={}, length={}, type={}", game::NET_AdrToString(to),
					ret, send_length, static_cast<int>(to.type_));
			}

			return ret;
		}
		*/
	}

	bool are_addresses_equal(const s4::netadr_t& a, const s4::netadr_t& b) {
		return net_compare_address(&a, &b);
	}

	const char* net_adr_to_string(const s4::netadr_t& a) {
		if (a.type_ == s4::netadrtype_t::NA_LOOPBACK) {
			return "loopback";
		}

		if (a.type_ == s4::netadrtype_t::NA_IP || a.type_ == s4::netadrtype_t::NA_BROADCAST) {
			if (a.port_) {
				return utils::string::va("%u.%u.%u.%u:%u", a.ip_[0], a.ip_[1], a.ip_[2], a.ip_[3], htons(a.port_));
			}

			return utils::string::va("%u.%u.%u.%u", a.ip_[0], a.ip_[1], a.ip_[2], a.ip_[3]);
		}

		return "bad";
	}

	void on(const std::string& command, const callback& callback) {
		get_callbacks()[utils::string::to_lower(command)] = callback;
	}

	void send(const s4::netadr_t& address, const std::string& command, const std::string& data, const char separator, std::optional<s4::netadrtype_t> adr_type_override) {
		std::string packet = "\xFF\xFF\xFF\xFF";
		packet.append(command);
		packet.push_back(separator);
		packet.append(data);
		packet.append("\xFF\xFF\xFF");

		send_data(address, packet, adr_type_override);
	}

	void send_data(const s4::netadr_t& address_in, const std::string& data, std::optional<s4::netadrtype_t> adr_type_override) {
		s4::netadr_t address{};
		utils::hook::copy(&address, &address_in, sizeof(s4::netadr_t));
		if (adr_type_override.has_value()) {
			address.type_ = adr_type_override.value();
		}

		lan_funcs::Sys_SendTo(query_socket, data.data(), static_cast<int>(data.size()), address);
	}

	struct component final : client_component {
		void post_load() override {
			scheduler::once<scheduler::pipeline::client>([] {
				assign_local_interface_addresses();
				lan_funcs::NET_OpenIP();
				if (query_socket != INVALID_SOCKET) {
					query_thread = std::jthread(receive_packets);
				}
			});
		}

		void pre_destroy() override {
			if (query_thread.joinable()) {
				query_thread.request_stop();
				query_thread.join();
			}

			if (query_socket != INVALID_SOCKET) {
				closesocket(query_socket);
				query_socket = INVALID_SOCKET;
				query_socket_port = 0;
			}
		}

		void post_unpack() override {
			utils::hook::jump(game::GetProtocolVersion, get_protocol_version_stub);

			on("lanPing", [](const s4::netadr_t& address, const std::string_view& data, const int) {
				send(address, "lanPong", std::string(data), ' ', s4::netadrtype_t::NA_RAW);
			});

			on("lanPong", [](const s4::netadr_t& address, const std::string_view& data, const int) {
				LOG("Component/Network", INFO, "LAN pong from {}: {}", address.to_string_type_ignorant(), data);
			});

			command::add("lan_status", [] {
				LOG("Component/Network", INFO, "PID {}, profile '{}', query port {}", GetCurrentProcessId(),
					utils::storage::get_client_folder(), query_socket_port);
			});

			command::add("lan_ping", [](const command::params_cl& params) {
				if (params.size() != 2) {
					LOG("Component/Network", INFO, "usage: lan_ping <ip:port>");
					return;
				}

				s4::netadr_t address{};
				if (query_socket == INVALID_SOCKET || !lan_funcs::NET_StringToAdr(params[1].data(), &address)) {
					LOG("Component/Network", WARN, "LAN query socket unavailable or invalid address");
					return;
				}

				send(address, "lanPing", std::to_string(GetCurrentProcessId()), ' ', s4::netadrtype_t::NA_RAW);
			});
		}
	};
}

REGISTER_COMPONENT(network::component)

#include "common.hpp"
#include "component/lan/lan_funcs.hpp"

#include "component/lan/network.hpp"
#include "game/dvars.hpp"
#include "game/game.hpp"

#include <utils/cryptography.hpp>
#include <utils/string.hpp>

namespace lan_funcs {
	const char* NET_ErrorString() {
#		define WS_ERROR_STR_DEF(n) { n, #n }
		static const std::unordered_map<int, const char*> ws_error_map = {
			WS_ERROR_STR_DEF(WSAEINTR),
			WS_ERROR_STR_DEF(WSAEBADF),
			WS_ERROR_STR_DEF(WSAEACCES),
			WS_ERROR_STR_DEF(WSAEFAULT),
			WS_ERROR_STR_DEF(WSAEINVAL),
			WS_ERROR_STR_DEF(WSAEMFILE),
			WS_ERROR_STR_DEF(WSAEWOULDBLOCK),
			WS_ERROR_STR_DEF(WSAEINPROGRESS),
			WS_ERROR_STR_DEF(WSAEALREADY),
			WS_ERROR_STR_DEF(WSAENOTSOCK),
			WS_ERROR_STR_DEF(WSAEDESTADDRREQ),
			WS_ERROR_STR_DEF(WSAEMSGSIZE),
			WS_ERROR_STR_DEF(WSAEPROTOTYPE),
			WS_ERROR_STR_DEF(WSAENOPROTOOPT),
			WS_ERROR_STR_DEF(WSAEPROTONOSUPPORT),
			WS_ERROR_STR_DEF(WSAESOCKTNOSUPPORT),
			WS_ERROR_STR_DEF(WSAEOPNOTSUPP),
			WS_ERROR_STR_DEF(WSAEPFNOSUPPORT),
			WS_ERROR_STR_DEF(WSAEAFNOSUPPORT),
			WS_ERROR_STR_DEF(WSAEADDRINUSE),
			WS_ERROR_STR_DEF(WSAEADDRNOTAVAIL),
			WS_ERROR_STR_DEF(WSAENETDOWN),
			WS_ERROR_STR_DEF(WSAENETUNREACH),
			WS_ERROR_STR_DEF(WSAENETRESET),
			WS_ERROR_STR_DEF(WSAECONNABORTED),
			WS_ERROR_STR_DEF(WSAECONNRESET),
			WS_ERROR_STR_DEF(WSAENOBUFS),
			WS_ERROR_STR_DEF(WSAEISCONN),
			WS_ERROR_STR_DEF(WSAENOTCONN),
			WS_ERROR_STR_DEF(WSAESHUTDOWN),
			WS_ERROR_STR_DEF(WSAETOOMANYREFS),
			WS_ERROR_STR_DEF(WSAETIMEDOUT),
			WS_ERROR_STR_DEF(WSAECONNREFUSED),
			WS_ERROR_STR_DEF(WSAELOOP),
			WS_ERROR_STR_DEF(WSAENAMETOOLONG),
			WS_ERROR_STR_DEF(WSAEHOSTDOWN),
			WS_ERROR_STR_DEF(WSAEHOSTUNREACH),
			WS_ERROR_STR_DEF(WSASYSNOTREADY),
			WS_ERROR_STR_DEF(WSAVERNOTSUPPORTED),
			WS_ERROR_STR_DEF(WSANOTINITIALISED),
			WS_ERROR_STR_DEF(WSAEDISCON),
			WS_ERROR_STR_DEF(WSAHOST_NOT_FOUND),
			WS_ERROR_STR_DEF(WSATRY_AGAIN),
			WS_ERROR_STR_DEF(WSANO_RECOVERY),
			WS_ERROR_STR_DEF(WSANO_DATA)
		};
#		undef WS_ERROR_STR_DEF

		int err = WSAGetLastError();
		if (auto it = ws_error_map.find(err); it != ws_error_map.end()) {
			return it->second;
		}
		return "UNKNOWN ERROR";
	}

	SOCKET NET_IPSocket(const char* net_interface_in, int src_port) {
		const char* net_interface = net_interface_in ? net_interface_in : "localhost";
		LOG("Component/LAN", INFO, "Opening IP socket: {}:{}", net_interface, src_port);

		auto socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (socket == INVALID_SOCKET) {
			if (WSAGetLastError() != WSAEAFNOSUPPORT) {
				LOG("Component/LAN", WARN, "UDP_OpenSocket: socket: {}", NET_ErrorString());
			}
			return INVALID_SOCKET;
		}

		DWORD ioctl_arg = 1;
		if (ioctlsocket(socket, FIONBIO, &ioctl_arg) == SOCKET_ERROR) {
			LOG("Component/LAN", WARN, "UDP_OpenSocket: ioctl FIONBIO: {}", NET_ErrorString());
			closesocket(socket);
			return INVALID_SOCKET;
		}

		DWORD sso_arg = 1;
		if (setsockopt(socket, SOL_SOCKET, SO_BROADCAST, PTR_AS(const char*, &sso_arg), sizeof(sso_arg)) == SOCKET_ERROR) {
			LOG("Component/LAN", WARN, "UDP_OpenSocket: setsockopt SO_BROADCAST: {}", NET_ErrorString());
			closesocket(socket);
			return INVALID_SOCKET;
		}

		sockaddr_in addr{};
		if (utils::string::to_lower(net_interface) != "localhost") {
			if (!Sys_StringToSockaddr(net_interface, PTR_AS(sockaddr*, &addr))) {
				closesocket(socket);
				return INVALID_SOCKET;
			}
		}
		else {
			addr.sin_addr.S_un.S_addr = 0;
		}
		addr.sin_port = static_cast<std::uint16_t>(src_port) == 0xFFFF ? 0 : htons(static_cast<std::uint16_t>(src_port));
		addr.sin_family = AF_INET;

		if (bind(socket, PTR_AS(sockaddr*, &addr), sizeof(addr)) != SOCKET_ERROR) {
			return socket;
		}

		LOG("Component/LAN", WARN, "UDP_OpenSocket: bind: {}", NET_ErrorString());
		closesocket(socket);
		return INVALID_SOCKET;
	}

	void NET_OpenIP() {
		if (network::query_socket != INVALID_SOCKET) {
			return;
		}

		auto net_ip = dvars::get_dvar_by_name("LSSSQNLLMK");
		for (auto port = network::query_socket_port_min; port <= network::query_socket_port_max; port++) {
			network::query_socket = NET_IPSocket(dvars::get_string(net_ip, "localhost"), port);
			if (network::query_socket != INVALID_SOCKET) {
				network::query_socket_port = static_cast<std::uint16_t>(port);
				return;
			}
		}

		LOG("Component/LAN", WARN, "Couldn't allocate IP/UDP port, LAN discovery will not work!");
	}

	bool NET_StringToAdr(const char* s, s4::netadr_t* addr, std::uint16_t default_port) {
		if (s == nullptr || addr == nullptr) {
			return false;
		}

		if (std::strcmp(s, "localhost") == 0) {
			std::memset(addr, 0, sizeof(*addr));
			addr->type_ = s4::netadrtype_t::NA_LOOPBACK;
			return true;
		}

		char base[1024];
		ZeroMemory(base, sizeof(base));
		utils::string::copy(base, s);

		base[sizeof(base) - 1] = '\0';

		char* port_str = std::strchr(base, ':');
		if (port_str) {
			*port_str = '\0';
			++port_str;
		}

		if (!Sys_StringToAdr(base, addr)) {
			addr->type_ = s4::netadrtype_t::NA_BAD;
			return false;
		}

		if (DEREF_PTR_AS(std::uint32_t, addr->ip_) == 0xFFFFFFFF) {
			addr->type_ = s4::netadrtype_t::NA_BAD;
			return 0;
		}

		int port = port_str ? std::atoi(port_str) : default_port;
		addr->port_ = static_cast<std::uint16_t>(port);

		return 1;
	}

	int Sys_RecvFrom(SOCKET socket_, s4::netadr_t* net_from, s4::msg_t* net_message) {
		if (socket_ == INVALID_SOCKET || net_from == nullptr || net_message == nullptr) {
			return -1;
		}

		sockaddr_in from;
		int from_len = 16;
		auto res = recvfrom(socket_, PTR_AS(char*, net_message->data_), net_message->max_size_, 0, PTR_AS(sockaddr*, &from), &from_len);
		if (res != SOCKET_ERROR) {
			game::SockadrToNetadr(PTR_AS(sockaddr*, &from), net_from);
			//LOG("Component/Network", DEBUG, "Received packet from {}: {}", net_from->to_string_type_ignorant(), utils::string::dump_hex(std::string(PTR_AS(char*, net_message->data_), res)));
		}
		return res;
	}

	int Sys_SendTo(SOCKET socket_, const char* data, int length, s4::netadr_t to) {
		if (socket_ == INVALID_SOCKET || data == nullptr) {
			return -1;
		}

		sockaddr_in addr;
		game::NetadrToSockadr(&to, PTR_AS(sockaddr*, &addr));
		//LOG("Component/Network", DEBUG, "Sending packet to {}: {}", to.to_string_type_ignorant(), utils::string::dump_hex(std::string(data, length)));
		return sendto(socket_, data, length, 0, PTR_AS(sockaddr*, &addr), sizeof(sockaddr_in));
	}

	bool Sys_StringToAdr(const char* s, s4::netadr_t* addr) {
		std::uint32_t ip = 0;

		if (!std::isdigit(static_cast<std::uint8_t>(s[0]))) {
			hostent* he = ::gethostbyname(s);
			if (!he || !he->h_addr_list || !he->h_addr_list[0]) {
				return false;
			}

			int count = 0;
			for (char** p = he->h_addr_list; *p; ++p) {
				++count;
			}

			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_int_distribution<> dist(0, count - 1);
			int idx = dist(gen);
			ip = DEREF_PTR_AS(std::uint32_t, he->h_addr_list[idx]);
		}
		else {
			ip = ::inet_addr(s);
		}

		addr->type_ = s4::netadrtype_t::NA_IP;
		std::memcpy(addr->ip_, &ip, sizeof(ip));
		addr->port_ = 0;

		return true;
	}

	ULONG Sys_StringToSockaddr(const char* s, sockaddr* s_adr) {
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
}

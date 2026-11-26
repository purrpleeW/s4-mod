#pragma once
#include "common.hpp"
#include "game/function_types.hpp"

namespace lan_funcs {
	const char* NET_ErrorString();
	SOCKET NET_IPSocket(const char* net_interface, int src_port);
	void NET_OpenIP();
	bool NET_StringToAdr(const char* s, s4::netadr_t* addr, std::uint16_t default_port = 26016);
	int Sys_RecvFrom(SOCKET socket_, s4::netadr_t* net_from, s4::msg_t* net_message);
	int Sys_SendTo(SOCKET socket_, const char* data, int length, s4::netadr_t to);
	bool Sys_StringToAdr(const char* s, s4::netadr_t* addr);
	ULONG Sys_StringToSockaddr(const char* s, sockaddr* s_adr);
}

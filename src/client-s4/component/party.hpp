#pragma once
#include "common.hpp"
#include "game/game.hpp"

namespace party {
	struct connection_state {
		s4::netadr_t host;
		s4::XSESSION_INFO session_info;
		std::string challenge;
		bool queued_online_custom_game;
	};

	s4::PartyData* get_active_party();
	std::string session_info_to_bytes(s4::XSESSION_INFO& session_info);
	bool session_info_from_bytes(s4::XSESSION_INFO* session_info, std::string bytes);
	void reset_server_connection_state();
	void connect(const s4::netadr_t& address);
	void send_heartbeat();
}

#pragma once
#include "common.hpp"
#include "game/game.hpp"

#include <utils/info_string.hpp>

namespace server_list {
	bool get_master_server(s4::netadr_t& address);
	void handle_info_response(const s4::netadr_t& address, const utils::info_string& info);

	int get_player_count();
	int get_server_count();
}

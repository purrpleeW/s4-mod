#pragma once
#include <string>

namespace identification::utility {
	std::string get_client_name();
	std::string get_game_name();
	std::string get_game_version_full();
	std::string get_game_version();
	std::string get_game_deltaless_version();
	std::uint8_t get_game_config();
	std::uint8_t get_game_mode();
}

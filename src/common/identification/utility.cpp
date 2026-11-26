#include "identification/utility.hpp"
#include "identification/game.hpp"

namespace identification::utility {
	std::string get_client_name() {
		return game::get_target_info_game().client_name;
	}

	std::string get_game_name() {
		return game::get_target_info_game().game_name;
	}

	std::string get_game_version_full() {
		return game::is_target_game() ? game::get_full_display_name() : "<unknown version>";
	}

	std::string get_game_version() {
		return game::is_target_game() ? game::get_version() : "<unknown version>";
	}

	std::string get_game_deltaless_version() {
		return game::is_target_game() ? game::get_version(false) : "<unknown version>";
	}

	std::uint8_t get_game_config() {
		return game::is_target_game() ? game::get_target_game().game_config : game::Ship;
	}

	std::uint8_t get_game_mode() {
		return game::is_target_game() ? game::get_target_game().game_mode : game::AIO;
	}
}

#pragma once
#include "common.hpp"
#include "game/game.hpp"

namespace gsc::scripting {
	using script_function = void(*)(s4::scr_entref_t);

	std::string find_token(std::uint32_t id);
	std::string find_token_single(std::uint32_t id);

	script_function get_function_by_index(std::uint32_t index);
	script_function find_function(const std::string& name, const bool prefer_global);
}

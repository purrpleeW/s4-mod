#pragma once
#include "common.hpp"
#include "game/game.hpp"

namespace localized_strings {
	void set(const std::string& key, const std::string& value);
	void unset(const std::string& key);
	void set_group(const std::string& owner, const std::unordered_map<std::string, std::string>& values);
}

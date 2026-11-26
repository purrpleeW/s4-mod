#pragma once

#include "component/assets/camo/camo_menu.hpp"

#include <string>
#include <unordered_set>

namespace assets::camo::ui {
	menu::overrides refresh(const std::unordered_set<std::string>& folders);
}

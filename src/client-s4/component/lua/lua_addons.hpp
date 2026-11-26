#pragma once
#include "common.hpp"
#include "component/lua/addons/ui/ui_editor/conditions.hpp"
#include "component/lua/addons/ui/utils/loot_utils.hpp"
#include "component/lua/addons/ui/utils/mp_lobby_utils.hpp"

namespace lua::addons {
	inline std::unordered_map<std::string, std::string> addon_map = {
		// { "ui/uiEditor/conditions.lua", lua_hook::addons::ui::ui_editor::conditions_lua },
			{ "ui/utils/LootUtils.lua", lua_hook::addons::ui::utils::loot_utils_lua },
			{ "ui/utils/MPLobbyUtils.lua", lua_hook::addons::ui::utils::mp_lobby_utils_lua }
	};
}
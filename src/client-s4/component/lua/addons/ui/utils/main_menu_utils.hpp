#pragma once
#include <string>

namespace lua_hook::addons::ui::utils {
	inline const std::string main_menu_utils_beta_lua = R"(
assert(MAIN_MENU and type(MAIN_MENU.GetTimeRemainingFunc) == "function",
	"Missing beta main-menu timer")
MAIN_MENU.GetTimeRemainingFunc = function()
	return function(controllerIndex)
		return 0, false, false, false
	end
end
)";
}

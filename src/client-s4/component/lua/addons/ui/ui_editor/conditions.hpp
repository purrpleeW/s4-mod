#pragma once
#include <string>

namespace lua_hook::addons::ui::ui_editor {
	inline const std::string conditions_beta_lua = R"(
assert(CONDITIONS and type(CONDITIONS.IsQuickPlayEnabled) == "function"
	and type(CONDITIONS.IsQuickplayFilterEnabled) == "function"
	and type(CONDITIONS.IsMPPrivateMatchEnabled) == "function",
	"Missing beta multiplayer menu conditions")

-- don't allow quickplay, beta doesn't support it and it breaks the menu
CONDITIONS.IsQuickPlayEnabled = function()
	return false
end

CONDITIONS.IsQuickplayFilterEnabled = function()
	return false
end

-- allow PM, doesn't appear by default
CONDITIONS.IsMPPrivateMatchEnabled = function()
	return true
end
)";

	inline std::string conditions_lua = R"(
CONDITIONS.IsBattlePassBundleTopperActive = function(a1)
	return false
end

CONDITIONS.IsBattlePassTopperActive = function()
	return false
end

CONDITIONS.IsDevelopmentBuildLocalPlayAllowed = function()
	return true
end

CONDITIONS.IsDevelopmentBuild = function()
	return not CONDITIONS.IsSingleplayer()
end
)";
}

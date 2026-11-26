#pragma once
#include <string>

namespace lua_hook::addons::ui::utils {
	inline std::string loot_utils_lua = R"(
LOOT.IsUnlocked = function(a1, a2, a3)
	return true
end

local LOOT_IsUnlockedWithID_Original = LOOT.IsUnlockedWithID
LOOT.IsUnlockedWithID = function(a1, a2)
	if a2 ~= nil then
		return true
	end
	return LOOT_IsUnlockedWithID_Original(a1, a2)
end

LOOT.IsLootVisibleForType = function(a1, a2, a3)
	return true
end
)";
}

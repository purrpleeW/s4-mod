#pragma once
#include <string>

namespace lua_hook::addons::ui::utils {
	inline std::string mp_lobby_utils_lua = R"(
Lobby.StartPrivateMatchDS = function(a1)
	if CONDITIONS.IsArenaGameType() then
		local memberCount = lfxCall("Engine.GetMemberCountInPrivateParty")
		local maxMembers = Lobby.MaxPlayerLimitForGunfight
		if maxMembers < memberCount then
			LUI.FlowManager.RequestPopupMenu(self, "EnteringGameModeWithHighPrivatePartyMemberCountWarning", true, event.controller, false, {
				currentCount = memberCount,
				maxCount = maxMembers
			})
		end
	end
	lfxCbuf("xpartygo")
end
)";
}
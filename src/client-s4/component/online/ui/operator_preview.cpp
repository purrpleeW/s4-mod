#include "common.hpp"
#include "component/lua/lua_hook.hpp"

#include <loader/component_loader.hpp>

namespace online::ui::operator_preview {
	namespace {
		constexpr std::string_view operator_preview_lua = R"(
if not Lobby or type(Lobby.GetModelDetailsFromPlayerCard) ~= "function" then
	return
end

local getModelDetails = Lobby.GetModelDetailsFromPlayerCard
local getPlayerCardData = Lobby.GetPlayerCardDataForPartyMember

if type(getPlayerCardData) == "function" then
	Lobby.GetPlayerCardDataForPartyMember = function(controllerIndex, memberData, ...)
		local playerCard = getPlayerCardData(controllerIndex, memberData, ...)
		if playerCard and memberData and memberData.isMyPlayer then
			playerCard.isMyPlayer = memberData.isMyPlayer:GetValue(controllerIndex)
		end
		return playerCard
	end
end

Lobby.GetModelDetailsFromPlayerCard = function(playerCard, controllerIndex, highResolution, ...)
	if not playerCard or not playerCard.isMyPlayer or not OPERATOR
		or type(OPERATOR.GetEquippedOperatorSkin) ~= "function" then
		return getModelDetails(playerCard, controllerIndex, highResolution, ...)
	end

	local ok, bodyModel, headModel = pcall(OPERATOR.GetEquippedOperatorSkin, controllerIndex)
	if not ok or type(bodyModel) ~= "string" or bodyModel == ""
		or type(headModel) ~= "string" or headModel == "" then
		return getModelDetails(playerCard, controllerIndex, highResolution, ...)
	end

	return bodyModel, headModel
end
)";
	}

	struct component final : generic_component {
		void post_load() override {
			if (identification::game::is("1.07.1", "1.10.0", "1.14.0", "1.16.0", "1.24.1-ES", "1.27.0")) {
				lua::hook::add_post_load_addon("ui/utils/MPLobbyUtils.lua", operator_preview_lua);
			}
		}
	};
}

REGISTER_COMPONENT(online::ui::operator_preview::component)

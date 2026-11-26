#include "common.hpp"
#include "component/lua/lua_hook.hpp"
#include "component/online/matchmaking.hpp"

#include <loader/component_loader.hpp>

namespace online::ui::custom_games {
	namespace {
		constexpr std::string_view custom_games_lua = R"(
if not Lobby or type(Lobby.StartPrivateMatchDS) ~= "function"
	or type(lfxCbuf) ~= "function" then
	return
end

local startPrivateMatchDS = Lobby.StartPrivateMatchDS

Lobby.StartPrivateMatchDS = function(controllerIndex, ...)
	if not IsPrivateMatch() or IsSystemLink() or not CONDITIONS.IsCoreMultiplayer()
		or CONDITIONS.IsArenaGameType() or CONDITIONS.IsArenaEvoGameType() then
		return startPrivateMatchDS(controllerIndex, ...)
	end

	lfxCbuf("mm_forceStartPrivateMatch")
end
)";
	}

	struct component final : generic_component {
		void post_unpack() override {
			if (online::matchmaking::supports_match_start()) {
				// this lua addon loads AFTER our other addon
				lua::hook::add_post_load_addon("ui/frontEnd/PrivateMatchLobbyButtons.lua", custom_games_lua);
			}
		}
	};
}

REGISTER_COMPONENT(online::ui::custom_games::component)

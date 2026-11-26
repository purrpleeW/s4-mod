#include "common.hpp"

#include "component/lua/lua_hook.hpp"
#include "component/lua/lua_vm.hpp"

#include <loader/component_loader.hpp>

namespace online::ui::progression {
	namespace {
		int get_season_rank_stub(s4::lua_State* lua_vm) {
			lua::vm::push_integer(lua_vm, 2500);
			return 1;
		}

		int get_max_stat_stub(s4::lua_State* lua_vm) {
			lua::vm::push_integer(lua_vm, INT_MAX);
			return 1;
		}
	}

	struct component final : generic_component {
		void post_load() override {
			lua::hook::create("OnlineProgression.GetPlayerXP", get_max_stat_stub);
			lua::hook::create("OnlineProgression.GetSeasonRank", get_season_rank_stub);
			lua::hook::create("OnlineProgression.GetSeasonXP", get_max_stat_stub);
			lua::hook::create("OnlineProgression.GetPlayerPrestigeLevelAtSeason", get_max_stat_stub);
		}
	};
}

REGISTER_COMPONENT(online::ui::progression::component)

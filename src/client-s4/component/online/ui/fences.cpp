#include "common.hpp"

#include "component/lua/lua_hook.hpp"
#include "component/lua/lua_vm.hpp"
#include "component/online/playlists.hpp"
#include "game/game.hpp"

#include <engine/s4/enums/FenceState.hpp>
#include <loader/component_loader.hpp>

namespace online::ui::fences {
	namespace {
		functions::NET_IsStartedT* NET_IsStarted{};

		bool should_allow_online() {
			return NET_IsStarted() && online::playlists::should_allow_online();
		}

		int get_battle_net_fence_state_stub(s4::lua_State* lua_vm) {
			lua::vm::push_integer(lua_vm, should_allow_online() ? 1 /* pass */ : 0 /* block [wait] */);
			return 1;
		}

		int get_sign_in_exchange_state_stub(s4::lua_State* lua_vm) {
			lua::vm::push_integer(lua_vm, 2);
			return 1;
		}

		int get_online_fence_state_stub(s4::lua_State* lua_vm) {
			lua::vm::push_enum(lua_vm, should_allow_online() ? s4::FenceState::FENCE_SUCCESS : s4::FenceState::FENCE_REQUESTED);
			lua::vm::push_integer(lua_vm, 0);
			return 2;
		}

		int get_patch_state_stub(s4::lua_State* lua_vm) {
			lua::vm::push_integer(lua_vm, 1);
			lua::vm::push_integer(lua_vm, 0);
			return 2;
		}
	}

	struct component final : generic_component {
		void post_load() override {
			lua::hook::create("Engine.GetBattleNetFenceState", get_battle_net_fence_state_stub);
			lua::hook::create("Commerce.GetSignInExchangeState", get_sign_in_exchange_state_stub);
			lua::hook::create("Fences.ODSFGetState", get_online_fence_state_stub);
			lua::hook::create("Fences.OCDSGetState", get_online_fence_state_stub);
			lua::hook::create("Fences.PatchGetState", get_patch_state_stub);

			lua::hook::set("Engine.IsUserSignedInToDemonware", true);
			lua::hook::set("Engine.IsNetworkConnected", true);
		}

		void find_signatures(memory::signature_store& batch) override {
			batch.add(SETUP_POINTER(NET_IsStarted), "E8 ? ? ? ? 84 C0 74 ? E8 ? ? ? ? 83 E8", SETUP_MOD(add(1).rip()));
		}
	};
}

REGISTER_COMPONENT(online::ui::fences::component)

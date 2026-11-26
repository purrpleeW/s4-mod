#include "common.hpp"
#include "game/game.hpp"

#include "component/lua/lua_vm.hpp"
#include "component/lua/lua_hook.hpp"

namespace lua::bridges::global {
	inline int lua_lfx_call(s4::lua_State* lua_vm) {
		if (vm::get_stack_size(lua_vm) < 1) {
			return 0;
		}

		const auto lua_func = hook::get_lua_bridge_function(vm::get_std_string(lua_vm, 1));
		if (!lua_func) {
			LOG("Component/Lua/Hook", ERROR, "Failed to get lua function: {}", lua_func.error());
			return 0;
		}

		int num_args = vm::get_stack_size(lua_vm) - 1;
		for (int i = 2; i <= num_args + 1; ++i) {
			vm::duplicate_stack_value(lua_vm, i);
		}
		vm::remove_stack_value(lua_vm, 1);

		return lua_func.value()(lua_vm);
	}

	inline int lua_lfx_cbuf(s4::lua_State* lua_vm) {
		if (vm::get_stack_size(lua_vm) < 1) {
			return 0;
		}

		game::Cbuf_AddText_AutoEncrypt(vm::get_std_string(lua_vm, 1));
		return 0;
	}

	inline int lua_lfx_dvar_hash(s4::lua_State* lua_vm) {
		vm::push_string(lua_vm, std::format("#x3{:X}", game::hash_dvar(vm::get_std_string(lua_vm, 1))).c_str());
		return 1;
	}
}

#include "common.hpp"
#include "component/lua/lua_vm.hpp"

#include <loader/component_loader.hpp>
#include <utils/hook.hpp>

namespace lua::vm {

	namespace {
		functions::lua_insertT* lua_insert{};
		functions::lua_pushbooleanT* lua_pushboolean{};
		functions::lua_pushcclosureT* lua_pushcclosure{};
		functions::lua_pushintegerT* lua_pushinteger{};
		functions::lua_pushstringT* lua_pushstring{};
		functions::lua_pushvalueT* lua_pushvalue{};
		functions::lua_removeT* lua_remove{};
		functions::lua_settopT* lua_settop{};
		functions::lua_tointeger32T* lua_tointeger32{};
		functions::lua_tolstringT* lua_tolstring{};
		functions::luaL_loadbufferT* luaL_loadbuffer{};
		functions::LuaShared_PCallT* LuaShared_PCall{};
		functions::LUI_CoD_InitT* LUI_CoD_Init{};
		functions::LUI_CoD_ShutdownT* LUI_CoD_Shutdown{};
	}

	void init_vm(bool frontend, bool error_recovery) {
		if (!LUI_CoD_Init) {
			LOG("Component/Lua/VM", WARN, "LUI_CoD_Init wasn't found, can't init VM");
			return;
		}

		return LUI_CoD_Init(frontend, error_recovery);
	}

	void quit_vm() {
		if (!LUI_CoD_Shutdown) {
			LOG("Component/Lua/VM", WARN, "LUI_CoD_Shutdown wasn't found, can't quit the VM");
			return;
		}

		return LUI_CoD_Shutdown();
	}

	std::int64_t get_stack_size(s4::lua_State* lua_vm) {
		return lua_vm->top_ - lua_vm->base_;
	}

	void pop_stack_value(s4::lua_State* lua_vm, int idx) {
		return lua_settop(lua_vm, -1 - idx);
	}

	void hoist_stack_value(s4::lua_State* lua_vm, int idx) {
		return lua_insert(lua_vm, idx);
	}

	void duplicate_stack_value(s4::lua_State* lua_vm, int idx) {
		return lua_pushvalue(lua_vm, idx);
	}

	void remove_stack_value(s4::lua_State* lua_vm, int idx) {
		return lua_remove(lua_vm, idx);
	}

	void set_stack_size(s4::lua_State* lua_vm, int size) {
		return lua_settop(lua_vm, size);
	}

	int get_integer(s4::lua_State* lua_vm, int idx) {
		return lua_tointeger32(lua_vm, idx);
	}

	const char* get_lstring(s4::lua_State* lua_vm, int idx, std::size_t* len) {
		return lua_tolstring(lua_vm, idx, len);
	}

	std::string get_std_string(s4::lua_State* lua_vm, int idx) {
		std::size_t size = 0;
		const char* data = lua_tolstring(lua_vm, idx, &size);
		return std::string(data, size);
	}

	void push_boolean(s4::lua_State* lua_vm, bool b) {
		return lua_pushboolean(lua_vm, VAL_AS(int, b));
	}

	void push_c_closure(s4::lua_State* lua_vm, int(*c_func)(s4::lua_State*), int n) {
		return lua_pushcclosure(lua_vm, c_func, n);
	}

	void push_integer(s4::lua_State* lua_vm, int n) {
		return lua_pushinteger(lua_vm, n);
	}

	void push_string(s4::lua_State* lua_vm, const char* str) {
		return lua_pushstring(lua_vm, str);
	}

	int up_value_index(int idx) {
		return detail::lua_globals_index - idx;
	}

	int load_buffer(s4::lua_State* lua_vm, const char* buf, std::size_t size, const char* name) {
		return luaL_loadbuffer(lua_vm, buf, size, name);
	}

	int load_string(s4::lua_State* lua_vm, const char* str) {
		return load_buffer(lua_vm, str, strlen(str), str);
	}

	bool call(s4::lua_State* lua_vm, int n_args, int n_results) {
		return LuaShared_PCall(lua_vm, n_args, n_results);
	}

	struct component final : generic_component {
		component_priority priority() const override {
			return component_priority::pointers;
		}

		void find_signatures(memory::signature_store& batch) override {
			batch.add(SETUP_POINTER(lua_insert), "48 63 C2 4C 8B C9 85 D2");

			batch.add(SETUP_POINTER(lua_pushboolean), "48 8B 41 ? 45 33 C0 85 D2");

			batch.add(SETUP_POINTER(lua_pushcclosure), "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B D9 49 63 F8 48 8B 49");

			batch.add(SETUP_POINTER(lua_pushinteger), "48 8B 41 ? C5 F8 57 C0 C4 E1 FB 2A C2");

			batch.add(SETUP_POINTER(lua_pushstring), "48 89 5C 24 ? 57 48 83 EC ? 48 8B FA 48 8B D9 48 85 D2 75 ? 48 8B 41");

			batch.add(SETUP_POINTER(lua_pushvalue), "40 53 48 83 EC ? 48 8B D9 E8 ? ? ? ? 48 8B 53 ? 48 8B 00");

			batch.add(SETUP_POINTER(lua_remove), "4C 8B C1 85 D2 7E ? 48 8B 41 ? 48 8B 49");

			batch.add(SETUP_POINTER(lua_settop), "40 53 48 83 EC ? 48 63 C2 48 8B D9 85 D2");

			batch.add(SETUP_POINTER(lua_tointeger32), "48 89 5C 24 ? 57 48 83 EC ? 8B FA 48 8B D9 E8 ? ? ? ? 85 C0 75");

			batch.add(SETUP_POINTER(lua_tolstring), "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 49 8B F8 8B DA 48 8B F1");

			batch.add(SETUP_POINTER(luaL_loadbuffer), "4C 8B DC 53 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 4D 85 C9");

			batch.add(SETUP_POINTER(LuaShared_PCall), "E8 ? ? ? ? 8B F8 85 C0 74 ? 4C 8D 44 24", SETUP_MOD(add(1).rip()));

			batch.add(SETUP_POINTER(LUI_CoD_Init), "E8 ? ? ? ? 83 3D ? ? ? ? ? 75 ? C7 05 ? ? ? ? ? ? ? ? E8 ? ? ? ? 48 8D 15", SETUP_MOD(add(1).rip()));

			batch.add(SETUP_POINTER(LUI_CoD_Shutdown), "E8 ? ? ? ? E8 ? ? ? ? 33 D2 0F B6 C8", SETUP_MOD(add(1).rip()));
		}
	};
}

REGISTER_COMPONENT(lua::vm::component)

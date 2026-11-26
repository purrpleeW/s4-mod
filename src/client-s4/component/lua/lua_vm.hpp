#pragma once
#include "common.hpp"
#include "game/function_types.hpp"
#include <memory/signature_store.hpp>

namespace lua::vm {
	namespace detail {
		inline constexpr auto lua_registry_index	= (-10000);
		inline constexpr auto lua_environ_index		= (-10001);
		inline constexpr auto lua_globals_index		= (-10002);

		inline constexpr auto lua_tnone 			= (-1);
		inline constexpr auto lua_tnil				= 0;
		inline constexpr auto lua_tboolean			= 1;
		inline constexpr auto lua_tlightuserdata	= 2;
		inline constexpr auto lua_tnumber			= 3;
		inline constexpr auto lua_tstring			= 4;
		inline constexpr auto lua_ttable			= 5;
		inline constexpr auto lua_tfunction			= 6;
		inline constexpr auto lua_tuserdata			= 7;
		inline constexpr auto lua_tthread			= 8;

		inline constexpr auto lua_multret 			= lua_tnone;
	}

	void init_vm(bool frontend, bool error_recovery);
	void quit_vm();

	std::int64_t get_stack_size(s4::lua_State* lua_vm);
	void pop_stack_value(s4::lua_State* lua_vm, int idx);
	void hoist_stack_value(s4::lua_State* lua_vm, int idx);
	void duplicate_stack_value(s4::lua_State* lua_vm, int idx);
	void remove_stack_value(s4::lua_State* lua_vm, int idx);
	void set_stack_size(s4::lua_State* lua_vm, int size);

	const char* get_lstring(s4::lua_State* s, int idx, std::size_t* len);
	std::string get_std_string(s4::lua_State* lua_vm, int idx);
	int get_integer(s4::lua_State* lua_vm, int idx);

	void push_boolean(s4::lua_State* lua_vm, bool b);
	void push_c_closure(s4::lua_State* lua_vm, int(*c_func)(s4::lua_State*), int n);
	void push_integer(s4::lua_State* lua_vm, int n);
	void push_string(s4::lua_State* lua_vm, const char* str);

	template <typename E> requires std::is_enum_v<E>
	void push_enum(s4::lua_State* lua_vm, E value) {
		return push_integer(lua_vm, ENUM_UNDER(value));
	}

	int up_value_index(int idx);

	int load_buffer(s4::lua_State* lua_vm, const char* buf, std::size_t size, const char* name);
	int load_string(s4::lua_State* lua_vm, const char* str);

	bool call(s4::lua_State* lua_vm, int n_args, int n_results);
}

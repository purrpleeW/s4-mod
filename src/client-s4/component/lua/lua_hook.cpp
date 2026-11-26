#include "common.hpp"

#include "component/lua/lua_hook.hpp"
#include "component/lua/lua_vm.hpp"
#include "component/lua/lua_addons.hpp"
#include "component/lua/addons/ui/utils/main_menu_utils.hpp"

#include "component/lua/bridges/bridge_global_lfx.hpp"

#include "game/game.hpp"

#include <loader/component_loader.hpp>
#include <memory/memory.hpp>
#include <utils/string.hpp>

namespace lua::hook {
	namespace {
		functions::Lua_CoD_PushFunctionToGlobalTableT* Lua_CoD_PushFunctionToGlobalTable{};
		functions::lua_loadxT* lua_loadx{};
		functions::luaL_openlibT* luaL_openlib{};
		functions::luaL_registerXHashT* luaL_registerXHash{};

		utils::hook::detour lua_l_register_xhash_hook;
		utils::hook::detour lua_l_open_lib_hook;
		utils::hook::detour lua_loadx_hook;
		utils::hook::detour lua_cod_push_function_to_global_table_hook;

		std::map<std::string, std::map<std::uint64_t, int(*)(s4::lua_State*)>> lua_functions{};
		std::vector<std::tuple<utils::hook::detour*, std::string, void*>> queued_hooks{};
		std::unordered_map<std::string, std::string> post_load_addons{};

		std::string beta_lua_name(const std::string& name) {
			// beta registers functions under mw2019-style obfuscated strings rather than the
			// XHashes used by ship: the decimal digits of the lua hash, mapped to A-J
			const auto parts = utils::string::split(name, '.');
			if (parts.size() != 2) {
				return name;
			}

			auto obfuscated = std::to_string(game::hash_lua_beta(parts[1]));
			for (auto& digit : obfuscated) {
				digit = static_cast<char>('A' + (digit - '0'));
			}

			return std::format("{}.{}", parts[0], obfuscated);
		}

		int lua_chunk_wrapper(s4::lua_State* l) {
			const char* extra_code = vm::get_lstring(l, vm::up_value_index(1), nullptr);

			int n_args = vm::get_stack_size(l);
			vm::duplicate_stack_value(l, vm::up_value_index(2));
			vm::hoist_stack_value(l, 1);

			int status = vm::call(l, n_args, vm::detail::lua_multret);
			if (status != 0) {
				const char* err_msg = vm::get_lstring(l, -1, nullptr);
				LOG("Component/Lua/Hook", WARN, "Failed to call Lua patch from wrapper: {}", err_msg ?: "unknown error");
				vm::pop_stack_value(l, 1);
				return 0;
			}

			int n_results = vm::get_stack_size(l);

			if (vm::load_string(l, extra_code)) {
				const char* err_msg = vm::get_lstring(l, -1, nullptr);
				LOG("Component/Lua/Hook", WARN, "Failed to call Lua patch from wrapper: {}", err_msg ?: "unknown error");
				vm::pop_stack_value(l, 1);
				return 0;
			}

			int new_top = vm::get_stack_size(l);
			int extras = new_top - n_results;
			if (extras > 0) {
				vm::set_stack_size(l, n_results);
			}

			return n_results;
		}

		int post_load_chunk_wrapper(s4::lua_State* l) {
			const auto* extra_code = vm::get_lstring(l, vm::up_value_index(1), nullptr);
			const auto n_args = vm::get_stack_size(l);
			vm::duplicate_stack_value(l, vm::up_value_index(2));
			vm::hoist_stack_value(l, 1);
			if (vm::call(l, n_args, vm::detail::lua_multret)) {
				LOG("Component/Lua/Hook", ERROR, "Failed to initialize Lua chunk: {}",
					vm::get_lstring(l, -1, nullptr) ?: "unknown error");
				vm::pop_stack_value(l, 1);
				return 0;
			}

			const auto n_results = vm::get_stack_size(l);
			const auto load_status = vm::load_string(l, extra_code);
			if (load_status || vm::call(l, 0, 0)) {
				LOG("Component/Lua/Hook", ERROR, "Failed to apply post-load Lua addon: {}",
					vm::get_lstring(l, -1, nullptr) ?: "unknown error");
				vm::set_stack_size(l, n_results);
				return n_results;
			}

			return n_results;
		}

		void wrap_post_load_chunk(s4::lua_State* l, const std::string& code) {
			vm::push_string(l, code.c_str());
			vm::duplicate_stack_value(l, -2);
			vm::push_c_closure(l, post_load_chunk_wrapper, 2);
			vm::remove_stack_value(l, -2);
		}

		void lua_l_register_xhash_stub(s4::lua_State* s, const char* lib_name, const s4::luaL_XHashReg* l) {
			if (lib_name != nullptr) {
				auto lib_name_str = std::string(lib_name);
				if (lua_functions.find(lib_name_str) == lua_functions.end()) {
					lua_functions[lib_name_str] = {};
				}

				for (s4::luaL_XHashReg* f = const_cast<s4::luaL_XHashReg*>(l); f && f->key_ != 0 && f->func_ != nullptr; ++f) {
					std::string full_name = std::format("{}.{:X}", lib_name, f->key_);
					std::uintptr_t func_addr = PTR_AS(std::uintptr_t, f->func_);
					auto func_ptr = f->func_;

					for (auto& [detour, name, target] : queued_hooks) {
						std::string name_rehashed = name;
						auto parts = utils::string::split(name_rehashed, '.');
						if (parts.size() == 2) {
							name_rehashed = std::format("{}.{:X}", parts[0], hash_scr_lua(parts[1]));
						}

						if (name == full_name || name_rehashed == full_name) {
							if (detour != nullptr) {
								if (detour->get_place() == nullptr) {
									detour->create(func_addr, target);
								}
							}
							else {
								utils::hook::jump(func_addr, target);
							}
							LOG("Component/Lua/Hook", DEBUG, "Hooked {}.", name);
							func_ptr = PTR_AS(int(*)(s4::lua_State*), target);
							break;
						}
					}

					lua_functions[lib_name_str][f->key_] = func_ptr;
				}
			}

			return lua_l_register_xhash_hook.invoke<void>(s, lib_name, l);
		}

		void lua_l_open_lib_stub(s4::lua_State* s, const char* lib_name, const s4::luaL_Reg* l, std::uint32_t n_up) {
			if (lib_name != nullptr) {
				for (s4::luaL_Reg* f = const_cast<s4::luaL_Reg*>(l); f; ++f) {
					if (f->name_ == nullptr) {
						break;
					}

					std::string full_name = std::format("{}.{}", lib_name, f->name_);
					std::uintptr_t func_addr = PTR_AS(std::uintptr_t, f->func_);

					for (auto& [detour, name, target] : queued_hooks) {
						if (name == full_name || (identification::game::is_config(identification::game::Beta)
							&& beta_lua_name(name) == full_name)) {
							if (detour != nullptr) {
								if (detour->get_place() == nullptr) {
									detour->create(func_addr, target);
								}
							}
							else {
								utils::hook::jump(func_addr, target);
							}
							break;
						}
					}
				}
			}

			return lua_l_open_lib_hook.invoke<void>(s, lib_name, l, n_up);
		}

		int lua_loadx_stub(s4::lua_State* l, const char*(*reader)(s4::lua_State*, void*, std::uint64_t*), void* data, const char* chunk_name,
			const char* mode)
		{
			auto res = lua_loadx_hook.invoke<int>(l, reader, data, chunk_name, mode);
			if (res != 0) {
				return res;
			}

			auto chunk_name_lower = utils::string::to_lower(chunk_name);
			if (identification::game::is_config(identification::game::Beta)) {
				const std::string* beta_code = nullptr;
				if (chunk_name_lower == "@ui/utils/mainmenuutils.lua") {
					beta_code = &lua_hook::addons::ui::utils::main_menu_utils_beta_lua;
				}
				else if (chunk_name_lower == "@ui/uieditor/conditions.lua") {
					beta_code = &lua_hook::addons::ui::ui_editor::conditions_beta_lua;
				}
				if (beta_code) {
					wrap_post_load_chunk(l, *beta_code);
					return res;
				}
			}
			if (const auto addon = post_load_addons.find(chunk_name_lower); addon != post_load_addons.end()) {
				wrap_post_load_chunk(l, addon->second);
				return res;
			}
			for (auto& [file_name, code] : addons::addon_map) {
				if (chunk_name_lower != utils::string::to_lower(std::format("@{}", file_name))) {
					continue;
				}

				vm::push_string(l, code.c_str());
				vm::duplicate_stack_value(l, -2);
				vm::push_c_closure(l, lua_chunk_wrapper, 2);
			}

			return res;
		}

		void lua_cod_push_function_to_global_table_stub(s4::lua_State* lua_vm, const char* function_name, int(*function_ptr)(s4::lua_State*)) {
			if (_stricmp(function_name, "DebugPrint") == 0) {
				// TODO: also make a debug print hook
				lua_cod_push_function_to_global_table_hook.invoke<void>(lua_vm, "lfxCall", bridges::global::lua_lfx_call);
				lua_cod_push_function_to_global_table_hook.invoke<void>(lua_vm, "lfxCbuf", bridges::global::lua_lfx_cbuf);
				lua_cod_push_function_to_global_table_hook.invoke<void>(lua_vm, "lfxDvarHash", bridges::global::lua_lfx_dvar_hash);
			}
			lua_cod_push_function_to_global_table_hook.invoke<void>(lua_vm, function_name, function_ptr);
		}

		std::expected<std::pair<std::string, std::uint64_t>, std::string> get_hash_and_table_of_lua_func(const std::string& name) {
			auto target_func_parts = utils::string::split(name, '.');
			if (target_func_parts.size() != 2) {
				return std::unexpected("Invalid Lua function name");
			}

			std::uint64_t target_hash = 0;
			if (target_func_parts.at(1).starts_with("#")) {
				if (target_func_parts.at(1).size() > 17) {
					return std::unexpected("Invalid Lua function hash");
				}
				target_hash = std::stoull(std::format("0x{}", target_func_parts.at(1).substr(1)), nullptr, 16);
			}
			else {
				target_hash = hash_scr_lua(target_func_parts.at(1));
			}

			return std::make_pair(target_func_parts.at(0), target_hash);
		}

		int lua_return_true_stub(s4::lua_State* lua_vm) {
			lua::vm::push_boolean(lua_vm, true);
			return 1;
		}

		int lua_return_false_stub(s4::lua_State* lua_vm) {
			lua::vm::push_boolean(lua_vm, true);
			return 1;
		}
	}

	std::expected<std::function<int(s4::lua_State*)>, std::string> get_lua_bridge_function(const std::string& name) {
		const auto lua_detail = get_hash_and_table_of_lua_func(name);
		if (!lua_detail) {
			return std::unexpected(lua_detail.error());
		}

		const auto [lua_table, lua_func_hash] = lua_detail.value();

		auto it_global_tables = lua_functions.find(lua_table);
		if (it_global_tables == lua_functions.end()) {
			return std::unexpected("Invalid Lua functions table name");
		}

		auto it_target_table = it_global_tables->second.find(lua_func_hash);
		if (it_target_table == it_global_tables->second.end() || it_target_table->second == nullptr) {
			return std::unexpected("Invalid Lua function name");
		}

		return it_target_table->second;
	}

	std::uint64_t hash_scr_lua(const std::string& str) {
		return game::hash_64a(str, 0xCBF29CE484222325, 0x100000001B3);
	}

	void create(const std::string& name, void* target) {
		queued_hooks.push_back({ nullptr, name, target });
	}

	void create(utils::hook::detour& detour, const std::string& name, void* target) {
		queued_hooks.push_back({ &detour, name, target });
	}

	void set(const std::string& name, bool state) {
		queued_hooks.push_back({ nullptr, name, state ? lua_return_true_stub : lua_return_false_stub });
	}

	void add_post_load_addon(std::string_view file_name, std::string_view code) {
		post_load_addons[utils::string::to_lower(std::format("@{}", file_name))] = std::string(code);
	}

	struct component final : generic_component {
		component_priority priority() const override {
			return component_priority::pointers;
		}

		void find_signatures(memory::signature_store& batch) override {
			batch.add(SETUP_POINTER(Lua_CoD_PushFunctionToGlobalTable), "48 89 5C 24 08 57 48 83 EC 20 49 8B D8 48 8B F9 E8 ? ? ? ? 45 33 C0 48 8B D3 48 8B CF E8 ? ? ? ?"
				" BA EE D8 FF FF 48");

			batch.add(SETUP_POINTER(lua_loadx), "4C 8B DC 53 57 48 81 EC ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C4 48 89 84 24 ? ? ? ? 49 89 53");

			if (identification::game::is_config(identification::game::Beta)) {
				batch.add(SETUP_POINTER(luaL_openlib), "48 89 5C 24 ? 55 56 41 56 48 83 EC ? 48 8B 41");
			}
			else {
				batch.add(SETUP_POINTER(luaL_registerXHash), "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 49 8B D8 48 8B F2 4C 8B C2");
			}
		}

		void post_unpack() override {
			if (identification::game::is_config(identification::game::Beta)) {
				lua_l_open_lib_hook.create(luaL_openlib, lua_l_open_lib_stub);
			}
			else {
				lua_l_register_xhash_hook.create(luaL_registerXHash, lua_l_register_xhash_stub);
			}

			lua_loadx_hook.create(lua_loadx, lua_loadx_stub);
			lua_cod_push_function_to_global_table_hook.create(Lua_CoD_PushFunctionToGlobalTable, lua_cod_push_function_to_global_table_stub);
		}
	};
}

REGISTER_COMPONENT(lua::hook::component)
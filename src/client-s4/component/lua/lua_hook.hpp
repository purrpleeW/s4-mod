#pragma once
#include "common.hpp"
#include <utils/hook.hpp>

#include <expected>
#include <engine/s4/lua_State.hpp>

namespace lua::hook {
	void create(utils::hook::detour& detour, const std::string& name, void* target);
	void create(const std::string& name, void* target);

	void set(const std::string& name, bool state);
	void add_post_load_addon(std::string_view file_name, std::string_view code);

	std::expected<std::function<int(s4::lua_State*)>, std::string> get_lua_bridge_function(const std::string& name);
	std::uint64_t hash_scr_lua(const std::string& str);
}
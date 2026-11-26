#pragma once
#include "common.hpp"
#include "game/game.hpp"

#include <xsk/gsc/engine/s4.hpp>

namespace gsc {
	extern std::unique_ptr<xsk::gsc::s4::context> gsc_ctx;

	s4::ScriptFile* find_script(s4::XAssetType type, const char* name, int allow_create_default);
	std::uint8_t* get_script_bytecode(s4::ScriptFile* script_file);
	int get_script_bytecode_len(const s4::ScriptFile* script_file);
	std::string get_real_script_name(const std::string& given_name);
	std::uint32_t get_main_handle(const std::string& name);

	void register_script(const std::string& path, int resource_id, bool allowed = true);
	void on_begin_scripts(const std::function<void()>& callback);
}

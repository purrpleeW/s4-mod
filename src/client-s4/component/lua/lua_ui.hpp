#pragma once
#include "common.hpp"
#include "game/function_types.hpp"
#include <memory/signature_store.hpp>

namespace lua::ui {
	s4::LocalClientNum_t get_client();

	std::uint16_t create_model_from_path(std::uint16_t parent_node_index, const char* path);
	std::uint16_t get_global_model();

	void model_set_integer(std::uint16_t node_index, int new_value);
	void model_set_string(std::uint16_t node_index, const char* new_value);

	void open_menu(const char* menu_name, bool is_immediate = false);
	void open_modal(const char* menu_name, bool is_immediate = false);
	void open_popup(const char* menu_name, bool is_immediate = false);
}

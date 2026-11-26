#pragma once
#include "common.hpp"
#include "game/function_types.hpp"

namespace console {
	void draw_text(const std::string& str, float x, float y, s4::vec4_t color);
	void draw_rect(s4::vec4_t dimensions, s4::vec4_t color);
	void draw_text_with_cursor(const std::string& str, float x, float y, int cursor_pos, s4::vec4_t color);

	s4::GfxFont* get_console_font();
	int get_font_height_in_px();
	float get_text_width(const std::string& str);
	std::pair<float, float> get_viewport_dimensions();
}
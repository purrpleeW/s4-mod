#pragma once
#include "console_shared.hpp"
#include <string>
#include <utility>

namespace console::detail {
	template <typename T, generic_vec4 V>
	struct rendering_provider {
		static T* get_console_font();
		static int get_font_height_in_px();
		static float get_text_width(const std::string& str);
		static std::pair<float, float> get_viewport_dimensions();

		static void draw_text(const std::string& str, float x, float y, V color_vec);
		static void draw_rect(V dimensions, V color_vec);
		static void draw_text_with_cursor(const std::string& str, float x, float y, int cursor_pos, V color_vec);
	};

	struct game_data_provider {
		static bool get_key_catch_state();
		static void toggle_key_catch_state();

		static void execute_cbuf(const std::string& data);

		static std::uint64_t generate_dvar_checksum(const std::string& data);
		static bool find_dvar_by_name(const std::string& data);
		static bool find_dvar_by_hash(std::uint64_t data);
		static std::string get_dvar_display_value(std::uint64_t hash);
		static std::string get_dvar_default_display_value(std::uint64_t hash);
		static std::string get_dvar_domain(std::uint64_t hash);
		static int get_dvar_line_count(std::uint64_t hash);
	};

	template <generic_vec4 V>
	struct settings_provider {
		static V get_input_box_color();
		static V get_input_title_color();
		static V get_input_text_color();

		static V get_hint_box_color();
		static V get_hint_match_description_color();
		static V get_hint_match_description_unavailable_color();
		static V get_hint_match_cmd_color();
		static V get_hint_match_dvar_color();

		static V get_dvar_display_value_color();
		static V get_dvar_default_value_color();
		static V get_dvar_domain_color();

		static V get_extended_output_box_color();
		static V get_extended_output_scrollbar_background_color();
		static V get_extended_output_scrollbar_slider_color();
		static V get_extended_output_text_color();
	};
}
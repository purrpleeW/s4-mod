#pragma once
#include "console_shared.hpp"
#include "console_providers.hpp"
#include "console_autocomplete.hpp"
#include "console_extended_output.hpp"

#include "logger/logger.hpp"

#include <version.hpp>
#include <format>
#include <chrono>
#include <ranges>

namespace console::detail {
	template <typename T, generic_vec4 V>
	class console_renderer {
		using renderer = rendering_provider<T, V>;
		using settings = settings_provider<V>;

		console_positioning screen_pos_;
		float current_x_ = 0.f;
		float current_y_ = 0.f;
		bool has_builtin_cursor_;
	public:
		explicit console_renderer(bool has_builtin_cursor) :
			screen_pos_{ .left = layout::screen_edge_padding, .top = layout::screen_edge_padding },
			has_builtin_cursor_(has_builtin_cursor)
		{}

		[[nodiscard]] const console_positioning& get_screen_pos() const noexcept {
			return this->screen_pos_;
		}

		[[nodiscard]] int calculate_visible_lines() const {
			auto font_ptr = renderer::get_console_font();
			if (!font_ptr) {
				return 0;
			}
			const auto font_height_px = renderer::get_font_height_in_px();
			const auto console_area_height = this->screen_pos_.bottom - this->current_y_;
			const auto console_usable_area = console_area_height - font_height_px;
			return VAL_AS(int, console_usable_area / font_height_px);
		}

		void resize_frame() {
			const auto [viewport_x, viewport_y] = renderer::get_viewport_dimensions();
			this->screen_pos_.right = viewport_x - layout::screen_edge_padding;
			this->screen_pos_.bottom = viewport_y - layout::screen_edge_padding;
		}

		void draw_input(const char* buffer, int cursor_position, const std::string& client_name) {
			this->current_x_ = this->screen_pos_.left + layout::screen_edge_padding;
			this->current_y_ = this->screen_pos_.top + layout::screen_edge_padding;

			const auto title = std::format("{}: {}> ", client_name, BUILD_VERSION_STRING);
			const auto title_width = renderer::get_text_width(title);
			const auto font_height = renderer::get_font_height_in_px();
			const auto text_color = settings::get_input_text_color();
			const auto max_line_width = this->screen_pos_.right - (this->current_x_ + title_width);

			auto buffer_copy = std::string(buffer);
			std::size_t line_count = 1;
			if (renderer::get_text_width(buffer_copy) > max_line_width) {
				std::vector<std::string> lines{};
				while (true) {
					if (buffer_copy.empty()) {
						break;
					}
					std::string current_line{};
					while (renderer::get_text_width(current_line) < max_line_width && !buffer_copy.empty()) {
						current_line += buffer_copy.at(0);
						buffer_copy = buffer_copy.substr(1);
					}
					if (renderer::get_text_width(current_line) > max_line_width) {
						buffer_copy = std::move(current_line.back()) + buffer_copy;
						current_line.pop_back();
					}
					lines.push_back(current_line);
				}
				buffer_copy = utils::string::join(lines, "\n");
				line_count = lines.size();
			}

			const auto _x = this->current_x_ - layout::screen_edge_padding;
			const auto _y = this->current_y_ - layout::screen_edge_padding;
			const auto _w = (this->screen_pos_.right - this->screen_pos_.left) -
				((this->current_x_ - layout::screen_edge_padding) - this->screen_pos_.left);
			const auto _h = VAL_AS(float, line_count * VAL_AS(std::size_t, font_height)) + 12.f;

			this->draw_box({ _x, _y, _w, _h }, settings::get_input_box_color());

			renderer::draw_text(title, this->current_x_, this->current_y_ + font_height, settings::get_input_title_color());
			this->current_x_ += title_width;

			if (this->has_builtin_cursor_) {
				renderer::draw_text_with_cursor(buffer_copy, this->current_x_, this->current_y_ + font_height, cursor_position, text_color);
			}
			else {
				this->draw_text_with_custom_cursor(buffer_copy, this->current_x_, this->current_y_ + font_height, cursor_position, text_color);
			}

			this->current_y_ += _h;
		}

		void draw_suggestions(autocomplete_engine& autocomplete) {
			autocomplete.clear_active_choice();

			if (autocomplete.has_overflow()) {
				this->draw_suggestion_overflow(autocomplete.suggestion_count());
			}
			else if (autocomplete.has_multiple()) {
				this->draw_suggestion_options(autocomplete);
			}
			else if (autocomplete.has_single()) {
				this->draw_suggestion_exact(autocomplete);
			}
		}

		void draw_extended_output(extended_output_manager& output) {
			output.access([this, &output](extended_output_queue& queue) {
				this->draw_extended_output_box();

				const auto x = this->screen_pos_.left + layout::screen_edge_padding;
				const auto y = this->current_y_ + layout::screen_edge_padding;
				const auto w = this->screen_pos_.right - layout::screen_edge_padding - x;
				const auto h = this->screen_pos_.bottom - layout::screen_edge_padding - y;

				this->draw_extended_output_scrollbar({ x, y, w, h }, queue, output);
				this->draw_extended_output_text(x, y, queue, output);
			});
		}
	private:
		void draw_horizontal_line(float x, float y, float length, V color) {
			renderer::draw_rect({ x, y, length, layout::box_border_thickness }, color);
		}

		void draw_vertical_line(float x, float y, float length, V color) {
			renderer::draw_rect({ x, y, layout::box_border_thickness, length }, color);
		}

		void draw_box(V dimensions, V color) {
			renderer::draw_rect(dimensions, color);

			V darkened_color{ color.r * .5f, color.g * .5f, color.b * .5f, color.a };

			this->draw_horizontal_line(dimensions.x, dimensions.y, dimensions.z, darkened_color);
			this->draw_horizontal_line(dimensions.x, dimensions.y + dimensions.w - layout::box_border_thickness, dimensions.z, darkened_color);
			this->draw_vertical_line(dimensions.x, dimensions.y, dimensions.w, darkened_color);
			this->draw_vertical_line(dimensions.x + dimensions.z - layout::box_border_thickness, dimensions.y, dimensions.w, darkened_color);
		}

		void draw_text_with_custom_cursor(const std::string& str, float x, float y, int cursor_pos, V color) {
			using namespace std::chrono_literals;
			static bool should_show_cursor = false;
			static auto last_cursor_update = std::chrono::high_resolution_clock::now();

			auto now = std::chrono::high_resolution_clock::now();
			if (now - last_cursor_update >= 500ms) {
				should_show_cursor ^= true;
				last_cursor_update = now;
			}

			renderer::draw_text(str, x, y, color);
			if (should_show_cursor) {
				const auto font_height = renderer::get_font_height_in_px();
				auto cursor_x = x + renderer::get_text_width(std::string(str, cursor_pos));
				cursor_x -= (font_height / 18.f) * 3.f;
				renderer::draw_text("|", cursor_x, y, color);
			}
		}

		void draw_hint_box(int line_count, float offset_y = 0.f) {
			const auto _x = this->current_x_ - layout::screen_edge_padding;
			const auto _y = this->current_y_ - 3.f + offset_y;
			const auto _w = (this->screen_pos_.right - this->screen_pos_.left) -
				((this->current_x_ - layout::screen_edge_padding) - this->screen_pos_.left);
			const auto _h = line_count * renderer::get_font_height_in_px() + 12.f;

			this->draw_box({ _x, _y, _w, _h }, settings::get_hint_box_color());
		}

		void draw_hint_text(int line_index, const std::string& str, V color, float offset_x = 0.f, float offset_y = 0.f) {
			const auto font_px = renderer::get_font_height_in_px();
			const auto _y = this->current_y_ + (font_px * (line_index + 1)) + 3.f + offset_y;
			renderer::draw_text(str, this->current_x_ + offset_x, _y, color);
		}

		void draw_suggestion_overflow(int count) {
			this->draw_hint_box(1);
			this->draw_hint_text(0,
				std::format("{} matches (too many to show here, press shift+tilde to open the extended output, press tab to print them all)", count),
				settings::get_hint_match_dvar_color()
			);
		}

		void draw_suggestion_options(autocomplete_engine& autocomplete) {
			const auto& suggestions = autocomplete.get_suggestions();
			this->draw_hint_box(suggestions.size());

			const auto offset = (this->screen_pos_.right - this->current_x_) / 4.f;

			for (const auto& [index, suggestion] : std::views::enumerate(suggestions)) {
				if (index == 0) {
					autocomplete.set_active_choice(suggestion.name);
				}
				this->draw_suggestion_entry(index, suggestion, offset);
			}
		}

		void draw_suggestion_exact(autocomplete_engine& autocomplete) {
			const auto& suggestion = autocomplete.get_suggestions().at(0);
			autocomplete.set_active_choice(suggestion.name);

			if (suggestion.kind == suggestion_kind::command) {
				this->draw_hint_box(1);
				this->draw_hint_text(0, suggestion.name, settings::get_hint_match_cmd_color());
			}
			else if (suggestion.kind == suggestion_kind::dvar) {
				this->draw_dvar_details(suggestion);
			}
		}

		void draw_suggestion_entry(int index, const suggestion_entry& suggestion, float offset) {
			if (suggestion.kind == suggestion_kind::command) {
				this->draw_hint_text(index, suggestion.name, settings::get_hint_match_cmd_color());
			}
			else if (suggestion.kind == suggestion_kind::dvar) {
				this->draw_hint_text(index, suggestion.name, settings::get_hint_match_dvar_color());
				this->draw_hint_text(index,
					game_data_provider::get_dvar_default_display_value(suggestion.checksum),
					settings::get_hint_match_dvar_color(), offset
				);

				this->draw_hint_text(index,
					suggestion.description.value_or("No description"),
					suggestion.description.has_value()
						? settings::get_hint_match_description_color()
						: settings::get_hint_match_description_unavailable_color(),
					offset * 1.5f
				);
			}
		}

		void draw_dvar_details(const suggestion_entry& suggestion) {
			this->draw_hint_box(2);
			this->draw_hint_text(0, suggestion.name, settings::get_hint_match_dvar_color());

			const auto offset_x = (this->screen_pos_.right - this->current_x_) / 4.f;
			this->draw_hint_text(0,
				game_data_provider::get_dvar_display_value(suggestion.checksum),
				settings::get_dvar_display_value_color(), offset_x
			);

			this->draw_hint_text(1, " default", settings::get_dvar_default_value_color());
			this->draw_hint_text(1,
				game_data_provider::get_dvar_default_display_value(suggestion.checksum),
				settings::get_dvar_default_value_color(), offset_x
			);

			const auto offset_y = 2 * renderer::get_font_height_in_px() + 14.f;
			const auto line_count = game_data_provider::get_dvar_line_count(suggestion.checksum);

			this->draw_hint_box(line_count, offset_y);
			this->draw_hint_text(0,
				suggestion.description.value_or("No description"),
				suggestion.description.has_value()
					? settings::get_hint_match_description_color()
					: settings::get_hint_match_description_unavailable_color(),
				0, offset_y
			);

			this->draw_hint_text(1,
				game_data_provider::get_dvar_domain(suggestion.checksum),
				settings::get_dvar_domain_color(), 0, offset_y
			);
		}

		void draw_extended_output_box() {
			this->draw_box({
				this->screen_pos_.left,
				this->current_y_,
				this->screen_pos_.right - this->screen_pos_.left,
				this->screen_pos_.bottom - this->current_y_
			}, settings::get_extended_output_box_color());
		}

		void draw_extended_output_scrollbar(V dimensions, extended_output_queue& queue, extended_output_manager& output) {
			const auto _x = (dimensions.x + dimensions.z) - 10.f;
			this->draw_box({ _x, dimensions.y, 10.f, dimensions.w }, settings::get_extended_output_scrollbar_background_color());

			auto _h = dimensions.w;
			auto _y = dimensions.y;

			const auto visible_lines = output.get_visible_line_count();
			if (queue.size() > visible_lines) {
				const auto percentage = VAL_AS(float, visible_lines) / queue.size();
				_h *= percentage;

				const auto remaining_space = dimensions.w - _h;
				const auto percentage_above = VAL_AS(float, output.get_display_offset()) / (queue.size() - visible_lines);
				_y += remaining_space * percentage_above;
			}

			this->draw_box({ _x, _y, 10.f, _h }, settings::get_extended_output_scrollbar_slider_color());
		}

		void draw_extended_output_text(float x, float y, extended_output_queue& queue, extended_output_manager& output) {
			const auto visible_lines = output.get_visible_line_count();
			const auto display_offset = output.get_display_offset();
			const auto font_height = renderer::get_font_height_in_px();

			auto _y = y;
			for (int ix = 0; ix < visible_lines; ix++) {
				_y = font_height + _y;

				const auto offset_idx = ix + display_offset;
				if (offset_idx >= queue.size()) {
					return;
				}

				renderer::draw_text(queue.at(offset_idx), x, _y, settings::get_extended_output_text_color());
			}
		}
	};
}
#pragma once
#include "common_core.hpp"

#include "console_shared.hpp"
#include "console_providers.hpp"
#include "console_state.hpp"
#include "console_input_buffer.hpp"
#include "console_history.hpp"
#include "console_autocomplete.hpp"
#include "console_extended_output.hpp"
#include "console_renderer.hpp"

#include "identification/utility.hpp"
#include "logger/logger.hpp"

#include <Windows.h>

namespace console::detail {
	template <typename T, generic_vec4 V, bool has_standalone_cursor>
	class console_provider {
		console_state_machine state_;
		input_buffer input_;
		input_history history_;
		autocomplete_engine autocomplete_;
		extended_output_manager output_;
		console_renderer<T, V> renderer_;

		std::string last_executable_input_;
	public:
		console_provider() : renderer_(has_standalone_cursor) {}

		void clear_console() {
			this->input_.reset();
			this->history_.clear();
			this->autocomplete_.reset();
			this->output_.clear();
			this->last_executable_input_.clear();
		}

		void print_extended_output(const std::string& text) {
			this->output_.push(text);
		}

		void draw_console() {
			this->renderer_.resize_frame();

			if (!game_data_provider::get_key_catch_state()) {
				return;
			}

			this->renderer_.draw_input(
				this->input_.data(),
				this->input_.cursor_position(),
				identification::utility::get_client_name()
			);

			this->output_.set_visible_line_count(this->renderer_.calculate_visible_lines());
			this->output_.assert_valid_scroll_region();
			if (this->state_.shows_extended_output()) {
				this->renderer_.draw_extended_output(this->output_);
			}

			const auto current_input = this->input_.get_executable_input();
			if (!current_input.empty()) {
				if (current_input != this->last_executable_input_) {
					this->autocomplete_.update(current_input);
					this->last_executable_input_ = current_input;
				}
				this->renderer_.draw_suggestions(this->autocomplete_);
			}
		}

		bool key_catcher_char_event_hook(const int local_client_num, const std::uint8_t key_event) {
			using namespace keys;

			if (key_event == K_GRAVE || key_event == '|' || key_event == '\\') {
				return false;
			}

			if (key_event > 0x7F) {
				return true;
			}

			if (!game_data_provider::get_key_catch_state()) {
				return true;
			}

			if (key_event == K_TAB && this->autocomplete_.has_active_suggestion()) {
				this->input_.set_autocomplete(
					this->autocomplete_.get_current_choice(),
					this->input_.has_command_prefix()
				);
				this->autocomplete_.reset();
				this->last_executable_input_.clear();
				return false;
			}

			if (key_event == K_CLIPBOARD) {
				const auto clipboard = utils::string::get_clipboard_data();
				if (!clipboard.empty()) {
					for (char c : clipboard) {
						this->key_catcher_char_event_hook(local_client_num, VAL_AS(std::uint8_t, c));
					}
				}
				return false;
			}

			if (key_event == K_CLEAR) {
				this->clear_console();
				return false;
			}

			if (key_event == K_BACKSPACE) {
				this->input_.backspace();
				return false;
			}

			if (key_event == K_CTRL_BACKSPACE) {
				this->input_.delete_word_back();
				return false;
			}

			if (key_event < 0x20 || key_event > 0x7E) {
				return false;
			}

			this->input_.insert_char(VAL_AS(char, key_event));
			return true;
		}

		bool key_catcher_key_event_hook(const int local_client_num, const std::uint8_t key_event, const bool down) {
			using namespace keys;

			// Grave/tilde key - toggle console
			if (key_event == K_GRAVE) {
				if (!down) {
					return false;
				}

				const bool shift_down = (GetAsyncKeyState(VK_LSHIFT) & 0x8000) != 0;
				if (shift_down) {
					if (!this->state_.is_visible()) {
						this->state_.toggle_visibility();
						this->reset_input_state();
					}
					this->state_.toggle_extended_output();
				} else {
					this->state_.toggle_visibility();
					this->reset_input_state();
				}
				return false;
			}

			if (!game_data_provider::get_key_catch_state()) {
				return true;
			}

			if (!down) {
				return false;
			}

			const bool ctrl_down = (GetAsyncKeyState(VK_LCONTROL) & 0x8000) != 0;
			// Navigation through history
			if (key_event == K_UPARROW) {
				if (auto entry = this->history_.navigate_up()) {
					this->input_.set_content(*entry);
					this->autocomplete_.reset();
					this->last_executable_input_.clear();
				}
				return false;
			}

			if (key_event == K_DOWNARROW) {
				if (auto entry = this->history_.navigate_down()) {
					this->input_.set_content(*entry);
				} else {
					this->input_.reset();
				}
				this->autocomplete_.reset();
				this->last_executable_input_.clear();
				return false;
			}

			// Cursor movement
			if (key_event == K_RIGHTARROW) {
				if (ctrl_down) {
					this->input_.move_cursor_word_right();
				}
				else {
					this->input_.move_cursor_right();
				}
				return false;
			}

			if (key_event == K_LEFTARROW) {
				if (ctrl_down) {
					this->input_.move_cursor_word_left();
				}
				else {
					this->input_.move_cursor_left();
				}
				return false;
			}

			// Scroll through output
			if (key_event == K_MWHEELUP || key_event == K_PGUP) {
				this->output_.scroll_up();
				return false;
			}

			if (key_event == K_MWHEELDOWN || key_event == K_PGDN) {
				this->output_.scroll_down();
				return false;
			}

			// Execute command
			if (key_event == K_ENTER) {
				this->execute_current_input();
				return false;
			}

			// Tab to print all suggestions
			if (key_event == K_TAB && this->input_.length() >= 2 && this->autocomplete_.has_overflow()) {
				this->print_all_suggestions();
				return false;
			}

			return true;
		}

		void register_command_iterator(const std::function<std::vector<std::string>()>& gcn) {
			this->autocomplete_.get_command_names_fn = gcn;
		}

		void register_suggestion(const suggestion_entry& entry) {
			this->autocomplete_.register_suggestion(entry);
		}

		void remove_suggestion(std::uint64_t suggestion_checksum) {
			this->autocomplete_.remove_suggestion(suggestion_checksum);
		}
	private:
		void reset_input_state() {
			this->input_.reset();
			this->autocomplete_.reset();
			this->last_executable_input_.clear();
		}

		void execute_current_input() {
			const auto executable = this->input_.get_executable_input();
			if (executable.empty()) {
				return;
			}

			game_data_provider::execute_cbuf(executable);
			this->history_.add_entry(this->input_.str());

			LOG("Component/Console", INFO, "> {}", this->input_.data());

			this->reset_input_state();
		}

		void print_all_suggestions() {
			LOG("Component/Console", INFO, "> {}", this->input_.data());
			for (const auto& suggestion : this->autocomplete_.get_suggestions()) {
				LOG("Component/Console", INFO, "\t{}", suggestion.name);
			}
		}
	};
}
#pragma once
#include "console_shared.hpp"
#include "console_providers.hpp"

namespace console::detail {
	class console_state_machine {
		console_state current_state_ = console_state::hidden;
	public:
		[[nodiscard]] console_state get_state() const noexcept {
			return this->current_state_;
		}

		[[nodiscard]] bool is_visible() const noexcept {
			return this->current_state_ != console_state::hidden;
		}

		[[nodiscard]] bool shows_extended_output() const noexcept {
			return this->current_state_ == console_state::extended_output;
		}

		void toggle_visibility() {
			if (this->current_state_ == console_state::hidden) {
				this->current_state_ = console_state::input_only;
				game_data_provider::toggle_key_catch_state();
			} else {
				this->current_state_ = console_state::hidden;
				game_data_provider::toggle_key_catch_state();
			}
		}

		void toggle_extended_output() {
			if (this->current_state_ == console_state::input_only) {
				this->current_state_ = console_state::extended_output;
			} else if (this->current_state_ == console_state::extended_output) {
				this->current_state_ = console_state::input_only;
			}
		}

		void hide() {
			if (this->current_state_ != console_state::hidden) {
				this->current_state_ = console_state::hidden;
				game_data_provider::toggle_key_catch_state();
			}
		}
	};
}
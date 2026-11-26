#pragma once
#include "console_shared.hpp"

#include <string>
#include <optional>

namespace console::detail {
	class input_history {
		extended_output_queue history_;
		int current_index_ = -1;
	public:
		void reset_navigation() noexcept {
			this->current_index_ = -1;
		}

		void clear() {
			this->history_.clear();
			this->current_index_ = -1;
		}

		void add_entry(const std::string& entry) {
			// Remove duplicate if navigating through history
			if (this->current_index_ != -1 && this->current_index_ < this->history_.size()) {
				if (this->history_.at(this->current_index_) == entry) {
					this->history_.erase(this->history_.begin() + this->current_index_);
				}
			}

			this->history_.push_front(entry);

			if (this->history_.size() > layout::max_history_size) {
				this->history_.erase(this->history_.begin() + layout::max_history_size);
			}

			this->current_index_ = -1;
		}

		[[nodiscard]] std::optional<std::string> navigate_up() {
			if (this->history_.empty()) {
				return std::nullopt;
			}

			if (++this->current_index_ >= this->history_.size()) {
				this->current_index_ = this->history_.size() - 1;
			}

			if (this->current_index_ >= 0 && this->current_index_ < this->history_.size()) {
				return this->history_.at(this->current_index_);
			}

			return std::nullopt;
		}

		[[nodiscard]] std::optional<std::string> navigate_down() {
			if (--this->current_index_ < -1) {
				this->current_index_ = -1;
			}

			if (this->current_index_ >= 0 && this->current_index_ < this->history_.size()) {
				return this->history_.at(this->current_index_);
			}

			return std::nullopt;
		}

		[[nodiscard]] int current_index() const noexcept {
			return this->current_index_;
		}
	};
}
#pragma once
#include "console_shared.hpp"

#include <string>
#include <string_view>
#include <algorithm>

namespace console::detail {
	template <std::size_t max_string_size>
	class constrained_string {
		std::string data_;
		std::size_t cursor_ = 0;

	public:
		constrained_string() {
			this->data_.reserve(max_string_size);
		}

		[[nodiscard]] static constexpr std::size_t max_size() noexcept { return max_string_size; }
		[[nodiscard]] std::size_t size() const noexcept { return this->data_.size(); }
		[[nodiscard]] std::size_t remaining_capacity() const noexcept { return max_string_size - this->data_.size(); }
		[[nodiscard]] bool empty() const noexcept { return this->data_.empty(); }
		[[nodiscard]] bool full() const noexcept { return this->data_.size() >= max_string_size; }

		[[nodiscard]] const std::string& str() const noexcept { return this->data_; }
		[[nodiscard]] std::string_view view() const noexcept { return this->data_; }
		[[nodiscard]] const char* c_str() const noexcept { return this->data_.c_str(); }
		[[nodiscard]] char operator[](std::size_t idx) const { return this->data_[idx]; }

		[[nodiscard]] std::size_t cursor_position() const noexcept { return this->cursor_; }
		[[nodiscard]] bool cursor_at_start() const noexcept { return this->cursor_ == 0; }
		[[nodiscard]] bool cursor_at_end() const noexcept { return this->cursor_ == this->data_.size(); }

		void move_cursor_left() noexcept {
			if (this->cursor_ > 0) {
				this->cursor_--;
			}
		}

		void move_cursor_right() noexcept {
			if (this->cursor_ < this->data_.size()) {
				this->cursor_++;
			}
		}

		void move_cursor_to_start() noexcept {
			this->cursor_ = 0;
		}

		void move_cursor_to_end() noexcept {
			this->cursor_ = this->data_.size();
		}

		void move_cursor_word_left() {
			while (this->cursor_ > 0 && std::isspace(this->data_[this->cursor_ - 1])) {
				this->cursor_--;
			}
			while (this->cursor_ > 0 && !std::isspace(this->data_[this->cursor_ - 1])) {
				this->cursor_--;
			}
		}

		void move_cursor_word_right() {
			while (this->cursor_ < this->data_.size() && !std::isspace(this->data_[this->cursor_])) {
				this->cursor_++;
			}
			while (this->cursor_ < this->data_.size() && std::isspace(this->data_[this->cursor_])) {
				this->cursor_++;
			}
		}

		// Modification
		void clear() noexcept {
			this->data_.clear();
			this->cursor_ = 0;
		}

		bool insert(char c) {
			if (this->full()) {
				return false;
			}

			this->data_.insert(this->data_.begin() + this->cursor_, c);
			this->cursor_++;
			return true;
		}

		bool insert(std::string_view text) {
			const auto chars_to_insert = std::min(text.size(), this->remaining_capacity());
			if (chars_to_insert == 0) {
				return false;
			}

			this->data_.insert(this->cursor_, text.substr(0, chars_to_insert));
			this->cursor_ += chars_to_insert;
			return chars_to_insert == text.size();
		}

		bool backspace() {
			if (this->cursor_ == 0) {
				return false;
			}

			this->data_.erase(this->cursor_ - 1, 1);
			this->cursor_--;
			return true;
		}

		bool delete_forward() {
			if (this->cursor_ >= this->data_.size()) {
				return false;
			}

			this->data_.erase(this->cursor_, 1);
			return true;
		}

		void delete_word_back() {
			if (this->cursor_ == 0) {
				return;
			}

			std::size_t end = this->cursor_;
			while (this->cursor_ > 0 && std::isspace(this->data_[this->cursor_ - 1])) {
				this->cursor_--;
			}
			while (this->cursor_ > 0 && !std::isspace(this->data_[this->cursor_ - 1])) {
				this->cursor_--;
			}

			this->data_.erase(this->cursor_, end - this->cursor_);
		}

		void clear_to_cursor() {
			this->data_.erase(0, this->cursor_);
			this->cursor_ = 0;
		}

		void clear_from_cursor() {
			this->data_.erase(this->cursor_);
		}

		void set(std::string_view text) {
			this->data_.assign(text.substr(0, std::min(text.size(), max_string_size)));
			this->cursor_ = this->data_.size();
		}

		void set_with_cursor(std::string_view text, std::size_t cursor_pos) {
			this->set(text);
			this->cursor_ = std::min(cursor_pos, this->data_.size());
		}
	};

	class input_buffer {
		constrained_string<layout::buffer_size - 1> buffer_;

	public:
		void reset() noexcept {
			this->buffer_.clear();
		}

		[[nodiscard]] const char* data() const noexcept {
			return this->buffer_.c_str();
		}

		[[nodiscard]] std::string_view view() const noexcept {
			return this->buffer_.view();
		}

		[[nodiscard]] const std::string& str() const noexcept {
			return this->buffer_.str();
		}

		[[nodiscard]] std::size_t length() const noexcept {
			return this->buffer_.size();
		}

		[[nodiscard]] bool empty() const noexcept {
			return this->buffer_.empty();
		}

		[[nodiscard]] int cursor_position() const noexcept {
			return this->buffer_.cursor_position();
		}

		[[nodiscard]] char first_char() const noexcept {
			return this->buffer_.empty() ? '\0' : this->buffer_[0];
		}

		[[nodiscard]] bool has_command_prefix() const noexcept {
			if (this->buffer_.size() < 2) {
				return false;
			}
			const char first = this->buffer_[0];
			return first == '/' || first == '\\';
		}

		[[nodiscard]] std::string get_executable_input() const {
			if (this->has_command_prefix()) {
				return std::string(this->buffer_.view().substr(1));
			}
			return this->buffer_.str();
		}

		void insert_char(char c) {
			this->buffer_.insert(c);
		}

		void insert_text(std::string_view text) {
			this->buffer_.insert(text);
		}

		void backspace() {
			this->buffer_.backspace();
		}

		void delete_forward() {
			this->buffer_.delete_forward();
		}

		void delete_word_back() {
			this->buffer_.delete_word_back();
		}

		void move_cursor_left() {
			this->buffer_.move_cursor_left();
		}

		void move_cursor_right() {
			this->buffer_.move_cursor_right();
		}

		void move_cursor_word_left() {
			this->buffer_.move_cursor_word_left();
		}

		void move_cursor_word_right() {
			this->buffer_.move_cursor_word_right();
		}

		void move_cursor_to_start() {
			this->buffer_.move_cursor_to_start();
		}

		void move_cursor_to_end() {
			this->buffer_.move_cursor_to_end();
		}

		void clear_to_cursor() {
			this->buffer_.clear_to_cursor();
		}

		void clear_from_cursor() {
			this->buffer_.clear_from_cursor();
		}

		void set_content(const std::string& content) {
			this->buffer_.set(content);
		}

		void set_autocomplete(const std::string& completion, bool preserve_prefix) {
			std::string new_content;

			if (preserve_prefix && !this->buffer_.empty()) {
				char prefix = this->buffer_[0];
				if (prefix == '\\' || prefix == '/') {
					new_content += prefix;
				}
			}

			new_content += completion;

			if (new_content.size() < this->buffer_.max_size()) {
				new_content += ' ';
			}

			this->buffer_.set(new_content);
		}
	};
}
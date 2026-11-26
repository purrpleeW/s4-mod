#pragma once
#include "console_shared.hpp"

#include "utils/concurrency.hpp"
#include "utils/string.hpp"

namespace console::detail {
	class extended_output_manager {
		utils::concurrency::container<extended_output_queue, std::recursive_mutex> output_;
		int visible_line_count_ = 0;
		int display_line_offset_ = 0;
	public:
		void set_visible_line_count(int count) noexcept {
			this->visible_line_count_ = count;
		}

		[[nodiscard]] int get_visible_line_count() const noexcept {
			return this->visible_line_count_;
		}

		[[nodiscard]] int get_display_offset() const noexcept {
			return this->display_line_offset_;
		}

		void clear() {
			this->display_line_offset_ = 0;
			this->output_.access([](extended_output_queue& queue) {
				queue.clear();
			});
		}

		void push(const std::string& text) {
			const auto lines = utils::string::split(text, '\n');
			for (const auto& line : lines) {
				this->push_line(line);
			}
		}

		void scroll_up() {
			this->output_.access([this](extended_output_queue& queue) {
				if (queue.size() > this->visible_line_count_ && this->display_line_offset_ > 0) {
					this->display_line_offset_--;
				}
			});
		}

		void scroll_down() {
			this->output_.access([this](extended_output_queue& queue) {
				if (queue.size() > this->visible_line_count_ && this->display_line_offset_ < queue.size() - this->visible_line_count_) {
					this->display_line_offset_++;
				}
			});
		}

		void assert_valid_scroll_region() {
			this->output_.access([this](extended_output_queue& queue) {
				if (queue.size() <= this->visible_line_count_ || this->display_line_offset_ >= queue.size() - this->visible_line_count_) {
					this->display_line_offset_ = std::max<int>(0, VAL_AS(int, queue.size()) - this->visible_line_count_);
				}
			});
		}

		template <typename Fn>
		void access(Fn&& fn) {
			this->output_.access(std::forward<Fn>(fn));
		}
	private:
		void push_line(const std::string& line) {
			this->output_.access([&, this](extended_output_queue& queue) {
				if (this->visible_line_count_ > 0 && this->display_line_offset_ == queue.size() - this->visible_line_count_) {
					this->display_line_offset_++;
				}

				queue.push_back(line);

				if (queue.size() > layout::max_output_lines) {
					queue.pop_front();
					if (this->display_line_offset_ > 0) {
						this->display_line_offset_--;
					}
				}
			});
		}
	};
}
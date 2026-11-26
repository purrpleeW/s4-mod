#pragma once

#include "common_core.hpp"

namespace command::detail {
	struct cl_cmd_args_provider{};
	struct sv_cmd_args_provider{};

	template <typename C>
	concept generic_cmd_args = requires(C c, int i) {
		{ c.nesting_ } -> std::convertible_to<int>;
		{ c.arg_v(i) } -> std::same_as<const char*>;
		{ c.argc_[0] } -> std::same_as<int&>;
	};

	template <typename T, generic_cmd_args C>
	struct cmd_args_provider {
		static C* get() noexcept;
	};

	template <typename T, generic_cmd_args C>
	class params {
		int arg_nesting_ = 0;
		using args = cmd_args_provider<T, C>;
	public:
		params() noexcept {
			this->arg_nesting_ = args::get()->nesting_;
		};

		[[nodiscard]] const int size() const noexcept {
			return args::get()->argc_[this->arg_nesting_];
		}

		[[nodiscard]] const int arg_count() const noexcept {
			return this->size() - 1;
		}

		[[nodiscard]] const std::string_view get_name() const noexcept {
			return this->get(0);
		}

		[[nodiscard]] const std::string_view get_arg(const int index) const noexcept {
			return this->get(index + 1);
		}

		template <typename N>
		[[nodiscard]] const N get_arg(const int index) const noexcept {
			N result = N();
			const std::string_view value = get_arg(index);
			const auto [_, ec] = std::from_chars(value.data(), value.data() + value.size(), result);
			return (ec == std::errc()) ? result : N();
		}

		template <>
		[[nodiscard]] const std::string get_arg(const int index) const noexcept {
			return this->get_arg(index).data();
		}

		[[nodiscard]] const std::string_view get(const int index) const noexcept {
			if (index >= this->size() || index < 0 || this->size() <= 0) {
				return "";
			}
			return args::get()->arg_v(index);
		}

		[[nodiscard]] std::string join(const int index) const noexcept {
			std::string result = {};
			for (auto i = index; i < this->size(); i++) {
				if (i > index) {
					result.append(" ");
				}
				result.append(this->get(i));
			}
			return result;
		}

		[[nodiscard]] const std::vector<std::string_view> get_all() const noexcept {
			std::vector<std::string_view> params_;
			params_.reserve(this->size());
			for (auto i = 0; i < this->size(); i++) {
				params_.push_back(this->get(i));
			}
			return params_;
		}

		[[nodiscard]] const std::string_view operator[](const int index) const noexcept {
			return this->get(index);
		}
	};
}
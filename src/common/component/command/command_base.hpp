#pragma once

#include "common_core.hpp"
#include "command_params.hpp"
#include "command_client_info.hpp"

#include "utils/string.hpp"

namespace command::detail {
	void main_handler();
	void add_raw(const char* name, void (*callback)());
	void remove_raw(const char* name);

	template <typename p_type, typename cip = std::nullptr_t>
	using cmd_handler_type = std::conditional_t<
		std::is_null_pointer_v<cip>,
		std::function<void(const p_type&)>,
		std::function<void(const cip&, const p_type&)>
	>;

	template <typename p_type, bool b_register, typename cip = std::nullptr_t>
	class cmd_bind_provider {
		std::unordered_map<std::string, cmd_handler_type<p_type, cip>> handlers_;
	protected:
		void add_impl(const char* name, cmd_handler_type<p_type, cip> callback, void (*stub)()) noexcept {
			const auto command = utils::string::to_lower(name);
			const auto [it, inserted] = handlers_.try_emplace(command, std::move(callback));
			if (inserted) {
				if constexpr (b_register) {
					add_raw(name, stub);
				}
			}
			else {
				it->second = std::move(callback);
			}
		}

		void remove_impl(const char* name) noexcept {
			const auto command = utils::string::to_lower(name);
			handlers_.erase(command);
			if constexpr (b_register) {
				remove_raw(name);
			}
		}

		void replace_impl(const char* name, cmd_handler_type<p_type, cip> callback, void (*stub)()) noexcept {
			this->remove_impl(name);
			this->add_impl(name, callback, stub);
		}
	public:
		const auto& get_handlers() const noexcept {
			return handlers_;
		}
	};

	template <typename p_type>
	class cl_cmd_bind_provider : public cmd_bind_provider<p_type, true> {
	public:
		void add(const char* name, const std::function<void(const p_type&)>& callback) {
			this->add_impl(name, callback, main_handler);
		}

		void add(const char* name, const std::function<void()>& callback) {
			this->add_impl(name, [cb = std::move(callback)](const p_type&) {
				cb();
			}, main_handler);
		}

		void remove(const char* name) {
			this->remove_impl(name);
		}

		void replace(const char* name, const std::function<void(const p_type&)>& callback) {
			this->remove(name);
			this->add(name, callback);
		}

		void replace(const char* name, const std::function<void()>& callback) {
			this->remove(name);
			this->add(name, callback);
		}
	};

	template <typename p_type>
	class cs_cmd_bind_provider : public cmd_bind_provider<p_type, false> {
	public:
		void add(const char* name, const std::function<void(const p_type&)>& callback) {
			this->add_impl(name, callback, nullptr);
		}

		void add(const char* name, const std::function<void()>& callback) {
			this->add_impl(name, [cb = std::move(callback)](const p_type&) {
				cb();
			}, nullptr);
		}

		void remove(const char* name) {
			this->remove_impl(name);
		}

		void replace(const char* name, const std::function<void(const p_type&)>& callback) {
			this->remove(name);
			this->add(name, callback);
		}

		void replace(const char* name, const std::function<void()>& callback) {
			this->remove(name);
			this->add(name, callback);
		}
	};

	template <typename p_type, typename cip>
	class sv_cmd_bind_provider : public cmd_bind_provider<p_type, true, cip> {
	public:
		void add_sv(const char* name, std::function<void(const cip&, const p_type&)> callback) {
			this->add_impl(name, callback, nullptr);
		}

		void remove_sv(const char* name) {
			this->remove_impl(name);
		}

		void replace_sv(const char* name, std::function<void(const cip&, const p_type&)> callback) {
			this->remove_sv(name);
			this->add_sv(name, callback);
		}
	};
}
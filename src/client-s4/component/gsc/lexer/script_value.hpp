#pragma once
#include "game/game.hpp"
#include "component/gsc/lexer/variable_value.hpp"

#include <utils/string.hpp>

namespace gsc::scripting {
	class function;
	class value_wrap;

	namespace {
		std::array<const char*, 28> var_typename = {
			"undefined",
			"object",
			"string",
			"localized string",
			"vector",
			"float",
			"int",
			"codepos",
			"precodepos",
			"function",
			"builtin function",
			"builtin method",
			"stack",
			"animation",
			"pre animation",
			"anim tree",
			"thread",
			"notify thread",
			"time thread",
			"child thread",
			"struct",
			"removed entity",
			"entity",
			"array",
			"removed thread",
			"<free>",
			"thread list",
			"endon list"
		};

		std::string get_typename(const s4::VariableValue& value) {
			const auto _ = game::ScriptContext_Server();
			if (value.type_ == s4::VariableType::VAR_POINTER) {
				/*
				const auto type = context->object_variable_value_[value.u_.uint_value_].w_.type_;
				return var_typename[ENUM_UNDER(type_)];
				*/
				return "<unable to obtain typename for pointer>";
			}
			return var_typename[ENUM_UNDER(value.type_)];
		}

		template <typename T>
		std::string get_c_typename() {
			auto& info = typeid(T);

			if (info == typeid(std::string)) {
				return "string";
			}

			if (info == typeid(const char*)) {
				return "string";
			}

			return info.name();
		}

		// because of clang tomfoolery, this is a thing now..
		template <>
		std::string get_c_typename<gsc::scripting::function>() {
			return "function";
		}
	}

	class script_value {
	public:
		script_value() = default;
		script_value(const s4::VariableValue& value);
		script_value(const value_wrap& value);

		script_value(int value);
		script_value(unsigned int value);
		script_value(bool value);

		script_value(float value);
		script_value(double value);

		script_value(const char* value);
		script_value(const std::string& value);

		script_value(const function& value);

		std::string type_name() const {
			return get_typename(this->get_raw());
		}

		std::string to_string() const;

		template <typename T>
		bool is() const;

		template <typename T>
		T as() const {
			if (!this->is<T>()) {
				const auto type = get_typename(this->get_raw());
				const auto c_type = get_c_typename<T>();
				throw std::runtime_error(std::format("has type '{}' but should be '{}'", type, c_type));
			}

			return get<T>();
		}

		template <typename T, typename I = int>
		T* as_ptr() {
			const auto value = this->as<I>();

			if (!value) {
				throw std::runtime_error("is null");
			}

			return reinterpret_cast<T*>(value);
		}

		const s4::VariableValue& get_raw() const;

		variable_value value_{};
	private:
		template <typename T>
		T get() const;
	};

	class value_wrap {
	public:
		value_wrap(const gsc::scripting::script_value& value, int argument_index);

		std::string to_string() const {
			return this->value_.to_string();
		}

		std::string type_name() const {
			return this->value_.type_name();
		}

		template <typename T>
		T as() const {
			try {
				return this->value_.as<T>();
			}
			catch (const std::exception& e) {
				throw std::runtime_error(utils::string::va("parameter %d %s", this->argument_index_, e.what()));
			}
		}

		template <typename T, typename I = int>
		T* as_ptr() {
			try {
				return this->value_.as_ptr<T>();
			}
			catch (const std::exception& e) {
				throw std::runtime_error(utils::string::va("parameter %d %s", this->argument_index_, e.what()));
			}
		}

		template <typename T>
		T is() const {
			return this->value_.is<T>();
		}

		const s4::VariableValue& get_raw() const {
			return this->value_.get_raw();
		}

		int argument_index_{};
		gsc::scripting::script_value value_;
	};
}

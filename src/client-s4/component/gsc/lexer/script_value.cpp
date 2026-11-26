#include "common.hpp"
#include "component/gsc/lexer/script_value.hpp"
#include "component/gsc/lexer/function.hpp"

#include <utils/safe_release.hpp>

namespace gsc::scripting {
	/***************************************************************
	 * Constructors
	 **************************************************************/
	script_value::script_value(const s4::VariableValue& value)
		: value_(value)
	{}

	script_value::script_value(const value_wrap& value)
		: value_(value.get_raw())
	{}

	script_value::script_value(const int value) {
		s4::VariableValue variable{};
		variable.type_ = s4::VariableType::VAR_INTEGER;
		variable.u_.int_value_ = value;

		this->value_ = variable;
	}

	script_value::script_value(const unsigned int value) {
		s4::VariableValue variable{};
		variable.type_ = s4::VariableType::VAR_INTEGER;
		variable.u_.uint_value_ = value;

		this->value_ = variable;
	}

	script_value::script_value(const bool value)
		: script_value(static_cast<unsigned>(value))
	{}

	script_value::script_value(const float value) {
		s4::VariableValue variable{};
		variable.type_ = s4::VariableType::VAR_FLOAT;
		variable.u_.float_value_ = value;

		this->value_ = variable;
	}

	script_value::script_value(const double value)
		: script_value(static_cast<float>(value))
	{}

	script_value::script_value(const char* value) {
		s4::VariableValue variable{};
		variable.type_ = s4::VariableType::VAR_STRING;
		variable.u_.string_value_ = game::SL_GetString(value, 0);

		const auto _ = utils::safe_release([&variable] {
			const auto context = game::ScriptContext_Server();
			game::RemoveRefToValue(context, variable.type_, variable.u_);
		});

		this->value_ = variable;
	}

	script_value::script_value(const std::string& value)
		: script_value(value.data())
	{}

	script_value::script_value(const function& value) {
		s4::VariableValue variable{};
		variable.type_ = s4::VariableType::VAR_FUNCTION;
		variable.u_.code_pos_value_ = value.get_pos();

		this->value_ = variable;
	}

	/***************************************************************
	 * Integer
	 **************************************************************/
	template <>
	bool script_value::is<int>() const {
		return this->get_raw().type_ == s4::VariableType::VAR_INTEGER;
	}

	template <>
	bool script_value::is<unsigned int>() const {
		return this->is<int>();
	}

	template <>
	bool script_value::is<bool>() const {
		return this->is<int>();
	}

	template <>
	int script_value::get() const {
		return this->get_raw().u_.int_value_;
	}

	template <>
	unsigned int script_value::get() const {
		return this->get_raw().u_.uint_value_;
	}

	template <>
	bool script_value::get() const {
		return this->get_raw().u_.uint_value_ != 0;
	}

	/***************************************************************
	 * Float
	 **************************************************************/
	template <>
	bool script_value::is<float>() const {
		return this->get_raw().type_ == s4::VariableType::VAR_FLOAT;
	}

	template <>
	bool script_value::is<double>() const {
		return this->is<float>();
	}

	template <>
	float script_value::get() const {
		return this->get_raw().u_.float_value_;
	}

	template <>
	double script_value::get() const {
		return static_cast<double>(this->get_raw().u_.float_value_);
	}

	/***************************************************************
	 * String
	 **************************************************************/
	template <>
	bool script_value::is<const char*>() const {
		return this->get_raw().type_ == s4::VariableType::VAR_STRING;
	}

	template <>
	bool script_value::is<std::string>() const {
		return this->is<const char*>();
	}

	template <>
	const char* script_value::get() const {
		return game::SL_ConvertToString(this->get_raw().u_.string_value_);
	}

	template <>
	std::string script_value::get() const {
		return this->get<const char*>();
	}

	/***************************************************************
	 * Struct
	 **************************************************************/
	template <>
	bool script_value::is<std::map<std::string, script_value>>() const {
		if (this->get_raw().type_ != s4::VariableType::VAR_POINTER) {
			return false;
		}

		/*
		const auto id = this->get_raw().u_.uint_value_;
		const auto context = game::ScriptContext_Server();
		const auto type = context->object_variable_value_[ENUM_UNDER(id)].w_.type_;

		return type == s4::VariableType::VAR_OBJECT;
		*/
		return false;
	}

	/***************************************************************
	 * Function
	 **************************************************************/
	template <>
	bool script_value::is<function>() const {
		return this->get_raw().type_ == s4::VariableType::VAR_FUNCTION;
	}

	template <>
	function script_value::get() const {
		return function(this->get_raw().u_.code_pos_value_);
	}

	/***************************************************************
	 *
	 **************************************************************/
	const s4::VariableValue& script_value::get_raw() const {
		return this->value_.get();
	}

	value_wrap::value_wrap(const gsc::scripting::script_value& value, int argument_index)
		: argument_index_(argument_index)
		, value_(value)
	{}

	std::string script_value::to_string() const {
		if (this->is<int>()) {
			return utils::string::va("%i", this->as<int>());
		}

		if (this->is<float>()) {
			return utils::string::va("%f", this->as<float>());
		}

		if (this->is<std::string>()) {
			return this->as<std::string>();
		}

		if (this->is<function>()) {
			const auto func = this->as<function>();
			return utils::string::va("[[ %s ]]", func.get_name().data());
		}

		return this->type_name();
	}
}

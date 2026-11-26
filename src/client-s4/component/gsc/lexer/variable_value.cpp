#include "common.hpp"
#include "component/gsc/lexer/variable_value.hpp"

#include "engine/s4/enums/VariableType.hpp"
#include "game/game.hpp"

namespace gsc::scripting {
	variable_value::variable_value(const s4::VariableValue& value) {
		this->assign(value);
	}

	variable_value::variable_value(const variable_value& other) noexcept {
		this->operator=(other);
	}

	variable_value::variable_value(variable_value&& other) noexcept {
		this->operator=(std::move(other));
	}

	variable_value& variable_value::operator=(const variable_value& other) noexcept {
		if (this != &other) {
			this->release();
			this->assign(other.value_);
		}

		return *this;
	}

	variable_value& variable_value::operator=(variable_value&& other) noexcept {
		if (this != &other) {
			this->release();
			this->value_ = other.value_;
			other.value_.type_ = s4::VariableType::VAR_UNDEFINED;
		}

		return *this;
	}

	variable_value::~variable_value() {
		this->release();
	}

	const s4::VariableValue& variable_value::get() const {
		return this->value_;
	}

	void variable_value::assign(const s4::VariableValue& value) {
		this->value_ = value;
		const auto context = game::ScriptContext_Server();
		game::AddRefToValue(context, this->value_.type_, this->value_.u_);
	}

	void variable_value::release() {
		if (this->value_.type_ != s4::VariableType::VAR_UNDEFINED) {
			const auto context = game::ScriptContext_Server();
			game::RemoveRefToValue(context, this->value_.type_, this->value_.u_);
			this->value_.type_ = s4::VariableType::VAR_UNDEFINED;
		}
	}
}

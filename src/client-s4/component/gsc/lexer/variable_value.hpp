#pragma once
#include "common.hpp"
#include "game/game.hpp"

namespace gsc::scripting {
	class variable_value {
	public:
		variable_value() = default;
		variable_value(const s4::VariableValue& value);
		variable_value(const variable_value& other) noexcept;
		variable_value(variable_value&& other) noexcept;

		variable_value& operator=(const variable_value& other) noexcept;
		variable_value& operator=(variable_value&& other) noexcept;

		~variable_value();

		const s4::VariableValue& get() const;
	private:
		void assign(const s4::VariableValue& value);
		void release();

		s4::VariableValue value_{ { 0 }, s4::VariableType::VAR_UNDEFINED };
	};
}

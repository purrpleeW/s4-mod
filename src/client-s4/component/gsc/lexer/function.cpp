#include "common.hpp"
#include "component/gsc/lexer/function.hpp"

#include "component/gsc/scripting.hpp"

namespace gsc::scripting {
	function::function(const char* pos)
		: pos_(pos)
	{}

	script_value function::get_raw() const {
		s4::VariableValue value;
		value.type_ = s4::VariableType::VAR_FUNCTION;
		value.u_.code_pos_value_ = this->pos_;
		return value;
	}

	const char* function::get_pos() const {
		return this->pos_;
	}

	std::string function::get_name() const {
		if (gsc::scripting::script_function_table_rev.contains(this->pos_)) {
			const auto& func = gsc::scripting::script_function_table_rev[this->pos_];
			return utils::string::va("%s::%s", func.first.data(), func.second.data());
		}
		return "unknown function";
	}
}

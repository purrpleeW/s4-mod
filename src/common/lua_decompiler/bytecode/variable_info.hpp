#pragma once
#include "common_core.hpp"
#include "lua_decompiler/bytecode_impl.hpp"

namespace lua_decompiler {
	struct bytecode::variable_info {
		enum class type {
			t_end,			// end of variable info
			t_for_idx,		// for numeric loop index
			t_for_stop,		// for numeric loop limit
			t_for_step,		// for numeric loop step
			t_for_gen,		// for generic loop generator
			t_for_state,	// for generic loop state
			t_for_ctl,		// for generic loop control
			t_str			// local variable name
		};

		type type_;
		std::string name_;
		bool is_parameter_ = false;
		std::uint32_t scope_begin_ = 0;
		std::uint32_t scope_end_ = 0;
	};
}

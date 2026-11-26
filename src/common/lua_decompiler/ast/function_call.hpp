#pragma once
#include "common_core.hpp"
#include "lua_decompiler/ast_impl.hpp"

#include "lua_decompiler/ast/expression.hpp"

namespace lua_decompiler {
	struct ast::function_call {
		expression* function_ = nullptr;
		std::vector<expression*> arguments_{};
		expression* multres_argument_ = nullptr;
		bool is_method_ = false;
		std::uint8_t return_count_ = 0;
	};
}

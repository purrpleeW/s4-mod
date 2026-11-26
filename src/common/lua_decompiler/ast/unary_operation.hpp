#pragma once
#include "common_core.hpp"
#include "lua_decompiler/ast_impl.hpp"

#include "lua_decompiler/ast/expression.hpp"

namespace lua_decompiler {
	struct ast::unary_operation {
		enum class type {
			t_minus,
			t_not,
			t_length
		};

		type type_;
		expression* operand_ = nullptr;
	};
}

#pragma once
#include "common_core.hpp"
#include "lua_decompiler/ast_impl.hpp"

#include "lua_decompiler/ast/expression.hpp"

namespace lua_decompiler {
	struct ast::binary_operation {
		enum class type {
			t_addition,
			t_subtraction,
			t_multiplication,
			t_division,
			t_exponentiation,
			t_modulo,
			t_concatenation,
			t_less_than,
			t_less_equal,
			t_greater_than,
			t_greater_equal,
			t_equal,
			t_not_equal,
			t_and,
			t_or
		};

		type type_;
		expression* left_operand_ = nullptr;
		expression* right_operand_ = nullptr;
	};
}

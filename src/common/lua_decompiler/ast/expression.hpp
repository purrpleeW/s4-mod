#pragma once
#include "common_core.hpp"
#include "lua_decompiler/ast_impl.hpp"

namespace lua_decompiler {
	struct ast::expression {
		enum class type {
			t_constant,
			t_vararg,
			t_function,
			t_variable,
			t_function_call,
			t_table,
			t_binary_operation,
			t_unary_operation
		};

		expression(const type& type_in) {
			this->set_type(type_in);
		}

		~expression() {
			this->delete_type();
		}

		void set_type(const type& type_in);
		void delete_type();

		type type_;
		union {
			constant* constant_ = nullptr;
			function* function_;
			variable* variable_;
			function_call* function_call_;
			table* table_;
			binary_operation* binary_operation_;
			unary_operation* unary_operation_;
			std::uint8_t return_count_;
		};
	};
}

#include "common_core.hpp"
#include "lua_decompiler/ast/expression.hpp"

#include "lua_decompiler/ast/binary_operation.hpp"
#include "lua_decompiler/ast/constant.hpp"
#include "lua_decompiler/ast/function_call.hpp"
#include "lua_decompiler/ast/table.hpp"
#include "lua_decompiler/ast/unary_operation.hpp"
#include "lua_decompiler/ast/variable.hpp"

namespace lua_decompiler {
	void ast::expression::set_type(const type& type_in) {
		this->delete_type();
		this->type_ = type_in;

		switch (this->type_) {
		case type::t_constant:
			this->constant_ = new constant;
			break;
		case type::t_vararg:
			this->return_count_ = 0;
			break;
		case type::t_variable:
			this->variable_ = new variable;
			break;
		case type::t_function_call:
			this->function_call_ = new function_call;
			break;
		case type::t_table:
			this->table_ = new table;
			break;
		case type::t_binary_operation:
			this->binary_operation_ = new binary_operation;
			break;
		case type::t_unary_operation:
			this->unary_operation_ = new unary_operation;
			break;
		default:
			break;
		}
	}

	void ast::expression::delete_type() {
		switch (this->type_) {
		case type::t_constant:
			delete this->constant_;
			this->constant_ = nullptr;
			break;
		case type::t_function:
			this->function_ = nullptr;
			break;
		case type::t_variable:
			delete this->variable_;
			this->variable_ = nullptr;
			break;
		case type::t_function_call:
			delete this->function_call_;
			this->function_call_ = nullptr;
			break;
		case type::t_table:
			delete this->table_;
			this->table_ = nullptr;
			break;
		case type::t_binary_operation:
			delete this->binary_operation_;
			this->binary_operation_ = nullptr;
			break;
		case type::t_unary_operation:
			delete this->unary_operation_;
			this->unary_operation_ = nullptr;
			break;
		default:
			break;
		}
	}
}

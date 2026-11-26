#pragma once
#include "common_core.hpp"
#include "lua_decompiler/ast_impl.hpp"

#include "lua_decompiler/ast/expression.hpp"
#include "lua_decompiler/ast/slot_scope.hpp"

namespace lua_decompiler {
	struct ast::variable {
		enum class type {
			t_slot,
			t_upvalue,
			t_global,
			t_table_index
		};

		type type_;
		std::uint8_t slot_ = 0;
		slot_scope** slot_scope_ = nullptr;
		std::string name_;
		expression* table_ = nullptr;
		expression* table_index_ = nullptr;
		bool is_multres_ = false;
		std::uint32_t multres_index_ = 0;
	};
}

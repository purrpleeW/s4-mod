#pragma once
#include "common_core.hpp"
#include "lua_decompiler/ast_impl.hpp"

#include "lua_decompiler/ast/expression.hpp"

namespace lua_decompiler {
	struct ast::table {
		struct field {
			expression* key_ = nullptr;
			expression* value_ = nullptr;
		};

		struct {
			std::vector<expression*> list_{};
			std::vector<field> fields_{};
		} constants_;
		std::vector<field> fields_{};
		std::uint32_t multres_index_ = 0;
		expression* multres_field_ = nullptr;
	};
}

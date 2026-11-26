#pragma once
#include "common_core.hpp"
#include "lua_decompiler/ast_impl.hpp"

namespace lua_decompiler {
	struct ast::slot_scope {
		slot_scope* slot_scope_ = this;
		std::vector<slot_scope**> merged_scopes_;
		std::string name_;
		std::uint32_t scope_begin_ = INVALID_ID;
		std::uint32_t scope_end_ = INVALID_ID;
		std::uint32_t usages_ = 0;
	};
}

#pragma once
#include "common_core.hpp"
#include "lua_decompiler/ast_impl.hpp"

namespace lua_decompiler {
	struct ast::local {
		std::vector<std::string> names_;
		std::uint8_t base_slot_ = 0;
		std::uint32_t scope_begin_ = INVALID_ID;
		std::uint32_t scope_end_ = INVALID_ID;
		bool exclude_block_ = false;
	};
}

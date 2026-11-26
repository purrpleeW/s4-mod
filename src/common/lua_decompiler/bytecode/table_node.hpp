#pragma once
#include "common_core.hpp"
#include "lua_decompiler/bytecode_impl.hpp"

#include "lua_decompiler/bytecode/table_constant.hpp"

namespace lua_decompiler {
	struct bytecode::table_node {
		table_constant key_;
		table_constant value_;
	};
}

#pragma once
#include "common_core.hpp"
#include "lua_decompiler/bytecode_impl.hpp"

namespace lua_decompiler {
	struct bytecode::number_constant {
		enum class type {
			t_int,	// integer constant
			t_num	// number constant
		};

		type type_;
		union {
			std::uint32_t integer_;
			std::uint64_t number_ = 0;
		};
	};
}

#pragma once
#include "common_core.hpp"
#include "lua_decompiler/bytecode_impl.hpp"

namespace lua_decompiler {
	struct bytecode::table_constant {
		enum class type {
			t_nil,		// primitive nil
			t_false,	// primitive false
			t_true,		// primitive true
			t_int,		// integer constant
			t_num,		// number constant
			t_xhash,	// cod xhash constant
			t_str		// string constant
		};

		type type_;
		union {
			std::uint32_t integer_;
			std::uint64_t number_ = 0;
			struct {
				std::uint64_t xhash_;
				std::uint8_t xhash_type_;
			};
		};
		std::string string_;
	};
}

#pragma once
#include "common_core.hpp"
#include "lua_decompiler/bytecode_impl.hpp"

#include "lua_decompiler/bytecode/table_constant.hpp"
#include "lua_decompiler/bytecode/table_node.hpp"

namespace lua_decompiler {
	struct bytecode::constant {
		enum class type {
			t_child,	// child prototype
			t_tab,		// table constant
			t_i64,		// signed integer cdata constant
			t_u64,		// unsigned integer cdata constant
			t_xhash,	// cod xhash constant
			t_complex,	// imaginary number cdata constant
			t_str		// string constant
		};

		type type_;
		const prototype* prototype_ = nullptr;
		std::vector<table_constant> array_;
		std::vector<table_node> table_;
		union {
			std::uint64_t cdata_ = 0;
			struct {
				std::uint64_t xhash_;
				std::uint8_t xhash_type_;
			};
		};
		std::string string_;
	};
}

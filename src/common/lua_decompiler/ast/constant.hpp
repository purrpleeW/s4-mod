#pragma once
#include "common_core.hpp"
#include "lua_decompiler/ast_impl.hpp"

namespace lua_decompiler {
	struct ast::constant {
		enum class type {
			t_nil,
			t_false,
			t_true,
			t_number,
			t_cdata_signed,
			t_cdata_unsigned,
			t_cdata_imaginary,
			t_xhash,
			t_string
		};

		type type_;
		union {
			double number_;
			std::int64_t int_signed_;
			std::uint64_t int_unsigned_ = 0;
			struct {
				std::uint64_t xhash_;
				std::uint8_t xhash_type_;
			};
		};
		std::string string_;
		bool is_name_ = false;
	};
}

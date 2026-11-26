#pragma once
#include "common_core.hpp"
#include "lua_decompiler/bytecode_impl.hpp"

#include "lua_decompiler/bytecode/opcode.hpp"

namespace lua_decompiler {
	struct bytecode::instruction {
		opcode type_;
		std::uint8_t a_ = 0;
		std::uint8_t b_ = 0;
		std::uint8_t c_ = 0;
		std::uint16_t d_ = 0;
	};
}

#pragma once
#include "common_core.hpp"
#include "lua_decompiler/bytecode_impl.hpp"

#include "lua_decompiler/bytecode/constant.hpp"
#include "lua_decompiler/bytecode/instruction.hpp"
#include "lua_decompiler/bytecode/number_constant.hpp"
#include "lua_decompiler/bytecode/table_constant.hpp"
#include "lua_decompiler/bytecode/variable_info.hpp"

namespace lua_decompiler {
	struct bytecode::prototype {
		prototype(const bytecode& bc);

		void operator()(std::vector<prototype*>& unlinked_prototypes);

		struct {
			std::uint8_t flags_ = 0;
			std::uint8_t parameters_ = 0;
			std::uint8_t frame_size_ = 0;
			bool has_debug_info_ = false;
			bool has_line_info_ = false;
			std::uint32_t first_line_ = 0;
			std::uint32_t line_count_ = 0;
		} header_;
		std::vector<instruction> instructions_;
		std::vector<std::uint16_t> upvalues_;
		std::vector<constant> constants_;
		std::vector<number_constant> number_constants_;
		std::vector<std::uint32_t> line_map_;
		std::vector<std::string> upvalue_names_;
		std::vector<variable_info> variable_infos_;
		std::uint32_t prototype_size_ = 0;
	private:
		void read_header();
		void read_instructions();
		void read_upvalues();
		void read_constants(std::vector<prototype*>& unlinked_prototypes);
		void read_number_constants();
		void read_debug_info();
		std::uint8_t get_next_byte();
		std::uint32_t get_uleb128();
		std::uint32_t get_uleb128_33();
		std::string get_string();
		table_constant get_table_constant();

		const bytecode& bc_;
	};
}

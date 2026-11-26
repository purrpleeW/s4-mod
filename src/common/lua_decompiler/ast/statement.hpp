#pragma once
#include "common_core.hpp"
#include "lua_decompiler/ast_impl.hpp"

#include "lua_decompiler/ast/variable.hpp"
#include "lua_decompiler/bytecode/opcode.hpp"

namespace lua_decompiler {
	struct ast::statement {
		enum class type {
			t_empty,
			t_instruction,
			t_return,
			t_condition,
			t_goto,
			t_numeric_for,
			t_generic_for,
			t_loop,
			t_break,
			t_declaration,
			t_assignment,
			t_function_call,
			t_if,
			t_else,
			t_while,
			t_repeat,
			t_do,
			t_label
		};

		statement(const type& type_in)
			: type_(type_in)
		{}

		type type_;

		struct {
			bytecode::opcode type_ = bytecode::opcode::op_invalid;
			std::uint8_t a_ = 0;
			std::uint8_t b_ = 0;
			std::uint8_t c_ = 0;
			std::uint16_t d_ = 0;
			std::uint32_t id_ = INVALID_ID;
			std::uint32_t target_ = INVALID_ID;
			std::uint32_t label_ = INVALID_ID;
		} instruction_;

		function* function_ = nullptr;
		std::vector<statement*> block_;
		local* locals_ = nullptr;

		struct {
			bool allow_slot_swap_ = false;
			bool swapped_ = false;
		} condition_;

		struct {
			void register_slots(expression*& expression) {
				this->open_slots_.emplace_back(&expression);
			}

			template <typename... Expressions>
			void register_slots(expression*& expression, Expressions*&... expressions) {
				this->open_slots_.emplace_back(&expression);
				return this->register_slots(expressions...);
			}

			bool is_potential_method_ = false;
			bool is_table_constructor_ = false;
			bool forward_declaration_ = false;
			constant_type allowed_constant_type_ = constant_type::t_number;
			std::vector<variable> variables_;
			std::vector<expression*> expressions_;
			std::vector<expression**> open_slots_;
			expression* multres_return_ = nullptr;
		} assignment_;
	};
}

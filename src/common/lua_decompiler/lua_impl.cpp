#include "common_core.hpp"
#include "lua_decompiler/lua_impl.hpp"

#include "lua_decompiler/ld_common.hpp"
#include "lua_decompiler/ast/binary_operation.hpp"
#include "lua_decompiler/ast/constant.hpp"
#include "lua_decompiler/ast/expression.hpp"
#include "lua_decompiler/ast/function.hpp"
#include "lua_decompiler/ast/function_call.hpp"
#include "lua_decompiler/ast/statement.hpp"
#include "lua_decompiler/ast/table.hpp"
#include "lua_decompiler/ast/unary_operation.hpp"
#include "lua_decompiler/ast/variable.hpp"
#include "lua_decompiler/bytecode/prototype.hpp"

namespace lua_decompiler {
	lua::lua(const bytecode& bytecode_in, const ast& ast_in, const std::string& file_path, const bool& force_overwrite, const bool& minimize_diffs,
			const bool& unrestricted_ascii)
		: file_path_(file_path)
		, bc_(bytecode_in)
		, ast_(ast_in)
		, force_overwrite_(force_overwrite)
		, minimize_diffs_(minimize_diffs)
		, unrestricted_ascii_(unrestricted_ascii)
	{}

	lua::~lua() {
		this->close_file();
	}

	void lua::operator()() {
		// print_progress_bar();
		this->prototype_data_left_ = this->bc_.prototypes_total_size_;
		this->write_header();
		if (this->ast_.chunk_->block_.size()) {
			this->write_block(*this->ast_.chunk_, this->ast_.chunk_->block_);
		}
		this->prototype_data_left_ -= this->ast_.chunk_->prototype_.prototype_size_;
		// print_progress_bar(bytecode.prototypesTotalSize - prototypeDataLeft, bytecode.prototypesTotalSize);
		this->create_file();
		this->write_file();
		this->close_file();
		// erase_progress_bar();
	}

	void lua::write_header() {
		if (!this->unrestricted_ascii_) {
			this->write(UTF8_BOM);
		}
		if (!this->bc_.header_.chunk_name_.size()) {
			return;
		}
		this->write("-- chunkname: ");
		this->write_string(this->bc_.header_.chunk_name_);
		this->write(NEW_LINE, NEW_LINE);
	}

	void lua::write_block(const ast::function& function, const std::vector<ast::statement*>& block) {
		std::vector<ast::statement*>* elseBlock;
		bool isFunctionDefinition;
		bool previousLineIsEmpty = true;

		if (!block.size()) {
			this->write_indent();
			this->write("-- block empty", NEW_LINE);
			return;
		}

		for (std::uint32_t i = 0; i < block.size(); i++) {
			if (!previousLineIsEmpty) {
				switch (block[i - 1]->type_) {
				case ast::statement::type::t_return:
					if (block[i]->type_ != ast::statement::type::t_return) {
						previousLineIsEmpty = true;
					}
					break;
				case ast::statement::type::t_goto:
				case ast::statement::type::t_break:
					if (block[i]->type_ != ast::statement::type::t_goto && block[i]->type_ != ast::statement::type::t_break) {
						previousLineIsEmpty = true;
					}
					break;
				case ast::statement::type::t_declaration:
					if (block[i]->type_ != ast::statement::type::t_declaration) {
						previousLineIsEmpty = true;
					}
					break;
				case ast::statement::type::t_assignment:
					if (block[i]->type_ != ast::statement::type::t_assignment) {
						previousLineIsEmpty = true;
					}
					break;
				case ast::statement::type::t_function_call:
					if (block[i]->type_ != ast::statement::type::t_function_call) {
						previousLineIsEmpty = true;
					}
					break;
				case ast::statement::type::t_label:
					if (block[i]->type_ != ast::statement::type::t_label) {
						previousLineIsEmpty = true;
					}
					break;
				case ast::statement::type::t_numeric_for:
				case ast::statement::type::t_generic_for:
				case ast::statement::type::t_if:
				case ast::statement::type::t_else:
				case ast::statement::type::t_while:
				case ast::statement::type::t_repeat:
				case ast::statement::type::t_do:
					previousLineIsEmpty = true;
					break;
				default:
					break;
				}
				if (previousLineIsEmpty) {
					this->write(NEW_LINE);
				}
			}

			switch (block[i]->type_) {
			case ast::statement::type::t_return:
				this->write_indent();
				if (i != block.size() - 1) {
					this->write("do ");
				}
				this->write("return");
				if (block[i]->assignment_.expressions_.size() || block[i]->assignment_.multres_return_) {
					this->write(" ");
					this->write_expression_list(block[i]->assignment_.expressions_, block[i]->assignment_.multres_return_);
				}
				if (i != block.size() - 1) {
					this->write(" end");
				}
				break;
			case ast::statement::type::t_goto:
				this->write_indent();
				this->write("goto ", function.labels_[block[i]->instruction_.label_].name_);
				break;
			case ast::statement::type::t_numeric_for:
				this->write_indent();
				this->write("for ");
				this->write_variable(block[i]->assignment_.variables_.back(), false);
				this->write(" = ");
				this->write_expression(*block[i]->assignment_.expressions_[0], false);
				this->write(", ");
				this->write_expression(*block[i]->assignment_.expressions_[1], false);
				if (block[i]->assignment_.expressions_.size() == 3) {
					this->write(", ");
					this->write_expression(*block[i]->assignment_.expressions_[2], false);
				}
				this->write(" do", NEW_LINE);
				this->indent_level_++;
				this->write_block(function, block[i]->block_);
				this->indent_level_--;
				this->write_indent();
				this->write("end");
				break;
			case ast::statement::type::t_generic_for:
				this->write_indent();
				this->write("for ");
				this->write_assignment(block[i]->assignment_.variables_, block[i]->assignment_.expressions_, " in ", false);
				this->write(" do", NEW_LINE);
				this->indent_level_++;
				this->write_block(function, block[i]->block_);
				this->indent_level_--;
				this->write_indent();
				this->write("end");
				break;
			case ast::statement::type::t_break:
				this->write_indent();
				if (i != block.size() - 1) {
					this->write("do ");
				}
				this->write("break");
				if (i != block.size() - 1) {
					this->write(" end");
				}
				break;
			case ast::statement::type::t_declaration:
				isFunctionDefinition = false;
				if (block[i]->assignment_.variables_.size() == 1
					&& block[i]->assignment_.expressions_.size() == 1
					&& block[i]->assignment_.expressions_.back()->type_ == ast::expression::type::t_function)
				{
					isFunctionDefinition = true;
					if (!block[i]->assignment_.expressions_.back()->function_->assignment_slot_is_upvalue_) {
						for (std::uint8_t j = block[i]->assignment_.expressions_.back()->function_->upvalues_.size(); j--;) {
							if ((*block[i]->assignment_.expressions_.back()->function_->upvalues_[j].slot_scope_)->name_ != (*block[i]->assignment_.variables_.back().slot_scope_)->name_) {
								continue;
							}
							isFunctionDefinition = false;
							break;
						}
						if (isFunctionDefinition) {
							for (std::uint32_t j = block[i]->assignment_.expressions_.back()->function_->used_globals_.size(); j--;) {
								if (*block[i]->assignment_.expressions_.back()->function_->used_globals_[j] != (*block[i]->assignment_.variables_.back().slot_scope_)->name_) {
									continue;
								}
								isFunctionDefinition = false;
								break;
							}
						}
					}
				}
				if (isFunctionDefinition) {
					if (!previousLineIsEmpty) {
						this->write(NEW_LINE);
					}
					this->write_indent();
					this->write("local function ");
					this->write_variable(block[i]->assignment_.variables_.back(), false);
					this->write_function_definition(*block[i]->assignment_.expressions_.back()->function_, false);
					if (i != block.size() - 1) {
						this->write(NEW_LINE, NEW_LINE);
						previousLineIsEmpty = true;
						continue;
					}
				}
				else {
					this->write_indent();
					this->write("local ");
					this->write_assignment(block[i]->assignment_.variables_, block[i]->assignment_.expressions_, " = ", false);
				}
				break;
			case ast::statement::type::t_assignment:
				isFunctionDefinition = false;

				if (block[i]->assignment_.variables_.size() == 1
					&& block[i]->assignment_.expressions_.size() == 1
					&& block[i]->assignment_.expressions_.back()->type_ == ast::expression::type::t_function)
				{
					for (ast::variable* variable = &block[i]->assignment_.variables_.back(); true; variable = variable->table_->variable_) {
						switch (variable->type_) {
						case ast::variable::type::t_slot:
						case ast::variable::type::t_upvalue:
						case ast::variable::type::t_global:
							isFunctionDefinition = true;
							break;
						case ast::variable::type::t_table_index:
							if (variable->table_->type_ == ast::expression::type::t_variable
								&& variable->table_index_->type_ == ast::expression::type::t_constant
								&& variable->table_index_->constant_->is_name_)
							{
								continue;
							}
						}

						break;
					}
				}

				if (isFunctionDefinition) {
					if (!previousLineIsEmpty) write(NEW_LINE);
					write_indent();
					write("function ");

					if (block[i]->assignment_.variables_.back().type_ == ast::variable::type::t_table_index
						&& block[i]->assignment_.expressions_.back()->function_->parameter_names_.size()
						&& block[i]->assignment_.expressions_.back()->function_->parameter_names_.front() == "self")
					{
						this->write_variable(*block[i]->assignment_.variables_.back().table_->variable_, false);
						this->write(":", block[i]->assignment_.variables_.back().table_index_->constant_->string_);
						this->write_function_definition(*block[i]->assignment_.expressions_.back()->function_, true);
					}
					else {
						this->write_variable(block[i]->assignment_.variables_.back(), false);
						this->write_function_definition(*block[i]->assignment_.expressions_.back()->function_, false);
					}
					if (i != block.size() - 1) {
						this->write(NEW_LINE, NEW_LINE);
						previousLineIsEmpty = true;
						continue;
					}
					break;
				}
				this->write_indent();
				this->write_assignment(block[i]->assignment_.variables_, block[i]->assignment_.expressions_, " = ", i);
				break;
			case ast::statement::type::t_function_call:
				this->write_indent();
				this->write_function_call(*block[i]->assignment_.expressions_.back()->function_call_, i);
				break;
			case ast::statement::type::t_if:
				this->write_indent();
				this->write("if ");
				this->write_expression(*block[i]->assignment_.expressions_.back(), false);
				this->write(" then", NEW_LINE);
				this->indent_level_++;
				this->write_block(function, block[i]->block_);
				this->indent_level_--;
				this->write_indent();
				if (i + 1 < block.size() && block[i + 1]->type_ == ast::statement::type::t_else) {
					i++;
					elseBlock = &block[i]->block_;
					while (true) {
						if (elseBlock->size() == 1 && elseBlock->front()->type_ == ast::statement::type::t_if) {
							this->write("elseif ");
							this->write_expression(*elseBlock->front()->assignment_.expressions_.back(), false);
							this->write(" then", NEW_LINE);
							this->indent_level_++;
							this->write_block(function, elseBlock->front()->block_);
							this->indent_level_--;
							this->write_indent();
						}
						else if (elseBlock->size() == 2
							&& elseBlock->front()->type_ == ast::statement::type::t_if
							&& elseBlock->back()->type_ == ast::statement::type::t_else)
						{
							this->write("elseif ");
							this->write_expression(*elseBlock->front()->assignment_.expressions_.back(), false);
							this->write(" then", NEW_LINE);
							this->indent_level_++;
							this->write_block(function, elseBlock->front()->block_);
							this->indent_level_--;
							this->write_indent();
							elseBlock = &elseBlock->back()->block_;
							continue;
						}
						else {
							this->write("else", NEW_LINE);
							this->indent_level_++;
							this->write_block(function, *elseBlock);
							this->indent_level_--;
							this->write_indent();
						}
						break;
					}
				}
				this->write("end");
				break;
			case ast::statement::type::t_while:
				this->write_indent();
				this->write("while ");
				this->write_expression(*block[i]->assignment_.expressions_.back(), false);
				this->write(" do", NEW_LINE);
				this->indent_level_++;
				this->write_block(function, block[i]->block_);
				this->indent_level_--;
				this->write_indent();
				this->write("end");
				break;
			case ast::statement::type::t_repeat:
				this->write_indent();
				this->write("repeat", NEW_LINE);
				this->indent_level_++;
				this->write_block(function, block[i]->block_);
				this->indent_level_--;
				this->write_indent();
				this->write("until ");
				this->write_expression(*block[i]->assignment_.expressions_.back(), false);
				break;
			case ast::statement::type::t_do:
				this->write_indent();
				this->write("do", NEW_LINE);
				this->indent_level_++;
				this->write_block(function, block[i]->block_);
				this->indent_level_--;
				this->write_indent();
				this->write("end");
				break;
			case ast::statement::type::t_label:
				this->write_indent();
				this->write("::", function.labels_[block[i]->instruction_.label_].name_, "::");
				break;
			default:
				throw nullptr;
			}

			this->write(NEW_LINE);
			previousLineIsEmpty = false;
		}
	}

	void lua::write_expression(const ast::expression& expression, const bool& use_parentheses, const bool& is_table_index) {
		/*
		static bool test_cache = false;
		if (!test_cache) {
			this->xhash_map_["0"][0xAED84C33EC5F01EA] = "Localize";
			this->xhash_map_["0"][0xC9E5BE3B4BBA4E0E] = "GetDvarBool";
			this->xhash_map_["3"][0xDDEA0BAE0D133630] = "gate_cp_access";
			test_cache = true;
		}
		*/

		std::uint32_t next_list_index, next_field_index;
		std::uint8_t operator_precedence, operand_precedence;
		bool parentheses, is_first_field, is_field_found;

		auto is_potential_xhash_table_index = is_table_index
			&& expression.type_ == ast::expression::type::t_constant
			&& expression.constant_->type_ == ast::constant::type::t_xhash;

		if (!is_potential_xhash_table_index) {
			if (is_table_index) {
				this->write("[");
			}
			if (use_parentheses) {
				this->write("(");
			}
		}

		switch (expression.type_) {
		case ast::expression::type::t_constant:
			switch (expression.constant_->type_) {
			case ast::constant::type::t_nil: this->write("nil"); break;
			case ast::constant::type::t_false: this->write("false"); break;
			case ast::constant::type::t_true: this->write("true"); break;
			case ast::constant::type::t_number: this->write_number(expression.constant_->number_); break;
			case ast::constant::type::t_cdata_signed: this->write(std::to_string(expression.constant_->int_signed_), "LL"); break;
			case ast::constant::type::t_cdata_unsigned: this->write(std::to_string(expression.constant_->int_unsigned_), "ULL"); break;
			case ast::constant::type::t_cdata_imaginary:
				this->write_number(expression.constant_->number_);
				this->write("i");
				break;
			case ast::constant::type::t_xhash: {
				std::string hash_prefix = std::format("t{:d}", expression.constant_->xhash_type_);
				std::string hash_code = "0";
				if (this->bc_.header_.version_ <= bytecode::version_82_t9) {
					hash_prefix = "";
					hash_code = "";
				}
				else if (this->bc_.header_.version_ <= bytecode::version_83_iw9) {
					hash_prefix = std::format("t{:d}", expression.constant_->xhash_type_);
					hash_code = "0";
					switch (expression.constant_->xhash_type_) {
					case 0:	// fnv
						hash_prefix = "x";
						hash_code = "0";
						break;
					case 1:	// dvar
						hash_prefix = "d";
						hash_code = "3";
						break;
					case 2:	// asset
						hash_prefix = "a";
						hash_code = "4";
						break;
					case 3:	// xh32
						hash_prefix = "x32";
						hash_code = "1";
						break;
					default:
						break;
					}
				}
				else if (this->bc_.header_.version_ <= bytecode::version_84_t10) {
					hash_prefix = std::format("t{:d}", expression.constant_->xhash_type_);
					switch (expression.constant_->xhash_type_) {
					case 0:	// fnv
						hash_prefix = "x";
						hash_code = "0";
						break;
					case 1:	// dvar
						hash_prefix = "d";
						hash_code = "3";
						break;
					case 2:	// script
						hash_prefix = "s";
						hash_code = "S";
						break;
					case 3:	// asset
						hash_prefix = "a";
						hash_code = "4";
						break;
					case 4:	// xh32
						hash_prefix = "x32";
						hash_code = "1";
						break;
					default:
						break;
					}
				}
				auto stringified = std::format("@{}\"#{}{}{:X}\"", hash_prefix, hash_code.empty() ? "" : "x", hash_code, expression.constant_->xhash_);
				auto hash_code_forced = hash_code.empty() ? "0" : hash_code;
				auto dehashed_table_index = false;
				if (auto it = this->xhash_map_.find(hash_code_forced); it != this->xhash_map_.end()) {
					if (auto it_x = it->second.find(expression.constant_->xhash_); it_x != it->second.end()) {
						if (hash_code_forced == "0"s && is_potential_xhash_table_index) {
							stringified = std::format("@{}", it_x->second);
							dehashed_table_index = true;
						}
						else {
							stringified = std::format("@{}\"{}\"", hash_prefix, it_x->second);
						}
					}
				}
				if (is_table_index) {
					if (dehashed_table_index) {
						this->write(".");
					}
					else {
						this->write("[");
					}
				}
				this->write(stringified);
				if (is_table_index && !dehashed_table_index) {
					this->write("]");
				}
				break;
			}
			case ast::constant::type::t_string:
				this->write("\"");
				this->write_string(expression.constant_->string_);
				this->write("\"");
				break;
			}
			break;
		case ast::expression::type::t_vararg:
			this->write("...");
			break;
		case ast::expression::type::t_function:
			this->write("function");
			this->write_function_definition(*expression.function_, false);
			break;
		case ast::expression::type::t_variable:
			this->write_variable(*expression.variable_, false);
			break;
		case ast::expression::type::t_function_call:
			this->write_function_call(*expression.function_call_, false);
			break;
		case ast::expression::type::t_table:
			if (!expression.table_->constants_.list_.size()
				&& !expression.table_->constants_.fields_.size()
				&& !expression.table_->fields_.size()
				&& !expression.table_->multres_field_)
			{
				this->write("{}");
				break;
			}

			this->write("{", NEW_LINE);
			this->indent_level_++;
			this->write_indent();
			next_list_index = 1;
			next_field_index = 0;
			is_first_field = true;

			if (expression.table_->constants_.list_.size() && expression.table_->constants_.list_.front()->constant_->type_ != ast::constant::type::t_nil) {
				this->write("[0] = ");
				this->write_expression(*expression.table_->constants_.list_.front(), false);
				is_first_field = false;
			}

			while (!expression.table_->multres_field_ || next_list_index < expression.table_->multres_index_) {
				if (next_list_index < expression.table_->constants_.list_.size() && expression.table_->constants_.list_[next_list_index]->constant_->type_ != ast::constant::type::t_nil) {
					if (!is_first_field) {
						this->write(",", NEW_LINE);
						this->write_indent();
					}
					this->write_expression(*expression.table_->constants_.list_[next_list_index], false);
					is_first_field = false;
					next_list_index++;
					continue;
				}
				is_field_found = false;
				for (std::uint32_t i = next_field_index; i < expression.table_->fields_.size(); i++) {
					if (expression.table_->fields_[i].key_->type_ != ast::expression::type::t_constant
						|| expression.table_->fields_[i].key_->constant_->type_ != ast::constant::type::t_number
						|| expression.table_->fields_[i].key_->constant_->number_ != next_list_index)
					{
						continue;
					}
					is_field_found = true;

					while (next_field_index < i) {
						if (!is_first_field) {
							this->write(",", NEW_LINE);
							this->write_indent();
						}
						if (expression.table_->fields_[next_field_index].key_->type_ == ast::expression::type::t_constant && expression.table_->fields_[next_field_index].key_->constant_->is_name_) {
							this->write(expression.table_->fields_[next_field_index].key_->constant_->string_);
						}
						else {
							this->write("[");
							this->write_expression(*expression.table_->fields_[next_field_index].key_, false);
							this->write("]");
						}
						this->write(" = ");
						this->write_expression(*expression.table_->fields_[next_field_index].value_, false);
						is_first_field = false;
						next_field_index++;
					}

					break;
				}
				if (is_field_found) {
					if (!is_first_field) {
						this->write(",", NEW_LINE);
						this->write_indent();
					}
					if (!expression.table_->multres_field_
						&& next_field_index == expression.table_->fields_.size() - 1
						&& !expression.table_->constants_.fields_.size()
						&& (!expression.table_->constants_.list_.size()
							|| next_list_index >= expression.table_->constants_.list_.size() - 1))
					{
						switch (expression.table_->fields_.back().value_->type_) {
						case ast::expression::type::t_vararg:
						case ast::expression::type::t_function_call:
							this->write_expression(*expression.table_->fields_.back().value_, true);
							break;
						default:
							this->write_expression(*expression.table_->fields_.back().value_, false);
							break;
						}
						next_list_index++;
						next_field_index++;
						break;
					}
					this->write_expression(*expression.table_->fields_[next_field_index].value_, false);
					next_field_index++;
				}
				else if (!expression.table_->multres_field_ && next_list_index >= expression.table_->constants_.list_.size()) {
					break;
				}
				else {
					if (!is_first_field) {
						this->write(",", NEW_LINE);
						this->write_indent();
					}
					this->write("nil");
				}
				is_first_field = false;
				next_list_index++;
			}

			for (std::uint32_t i = next_list_index; i < expression.table_->constants_.list_.size(); i++) {
				if (expression.table_->constants_.list_[i]->constant_->type_ == ast::constant::type::t_nil) {
					continue;
				}
				if (!is_first_field) {
					this->write(",", NEW_LINE);
					this->write_indent();
				}
				this->write("[", std::to_string(i), "] = ");
				this->write_expression(*expression.table_->constants_.list_[i], false);
				is_first_field = false;
			}

			for (std::uint32_t i = 0; i < expression.table_->constants_.fields_.size(); i++) {
				if (!is_first_field) {
					this->write(",", NEW_LINE);
					this->write_indent();
				}
				if (expression.table_->constants_.fields_[i].key_->constant_->is_name_) {
					this->write(expression.table_->constants_.fields_[i].key_->constant_->string_);
				}
				else {
					this->write("[");
					this->write_expression(*expression.table_->constants_.fields_[i].key_, false);
					this->write("]");
				}
				this->write(" = ");
				this->write_expression(*expression.table_->constants_.fields_[i].value_, false);
				is_first_field = false;
			}

			for (std::uint32_t i = next_field_index; i < expression.table_->fields_.size(); i++) {
				if (!is_first_field) {
					this->write(",", NEW_LINE);
					this->write_indent();
				}
				if (expression.table_->fields_[i].key_->type_ == ast::expression::type::t_constant && expression.table_->fields_[i].key_->constant_->is_name_) {
					this->write(expression.table_->fields_[i].key_->constant_->string_);
				}
				else {
					this->write("[");
					this->write_expression(*expression.table_->fields_[i].key_, false);
					this->write("]");
				}
				this->write(" = ");
				this->write_expression(*expression.table_->fields_[i].value_, false);
				is_first_field = false;
			}

			if (expression.table_->multres_field_) {
				if (!is_first_field) {
					this->write(",", NEW_LINE);
					this->write_indent();
				}
				this->write_expression(*expression.table_->multres_field_, false);
			}

			this->indent_level_--;
			if (this->minimize_diffs_) {
				this->write(",");
			}
			this->write(NEW_LINE);
			this->write_indent();
			this->write("}");
			break;
		case ast::expression::type::t_binary_operation:
			operator_precedence = this->get_operator_precedence(expression);
			operand_precedence = this->get_operator_precedence(*expression.binary_operation_->left_operand_);
			parentheses = false;

			if (operand_precedence == operator_precedence) {
				switch (operand_precedence) {
				case 3:
				case 7:
					parentheses = true;
				}
			}
			else if (operand_precedence < operator_precedence) {
				parentheses = true;
			}
			else if (operator_precedence == 7 && expression.binary_operation_->left_operand_->type_ == ast::expression::type::t_constant) {
				switch (expression.binary_operation_->left_operand_->constant_->type_) {
				case ast::constant::type::t_number:
				case ast::constant::type::t_cdata_imaginary:
					if (std::bit_cast<std::uint64_t>(expression.binary_operation_->left_operand_->constant_->number_) & DOUBLE_SIGN) {
						parentheses = true;
					}
					break;
				case ast::constant::type::t_cdata_signed:
					if (expression.binary_operation_->left_operand_->constant_->int_signed_ < 0) {
						parentheses = true;
					}
					break;
				default:
					break;
				}
			}

			this->write_expression(*expression.binary_operation_->left_operand_, parentheses);

			switch (expression.binary_operation_->type_) {
			case ast::binary_operation::type::t_addition: this->write(" + "); break;
			case ast::binary_operation::type::t_subtraction: this->write(" - "); break;
			case ast::binary_operation::type::t_multiplication: this->write(" * "); break;
			case ast::binary_operation::type::t_division: this->write(" / "); break;
			case ast::binary_operation::type::t_exponentiation: this->write("^"); break;
			case ast::binary_operation::type::t_modulo: this->write(" % "); break;
			case ast::binary_operation::type::t_concatenation: this->write(" .. "); break;
			case ast::binary_operation::type::t_less_than: this->write(" < "); break;
			case ast::binary_operation::type::t_less_equal: this->write(" <= "); break;
			case ast::binary_operation::type::t_greater_than: this->write(" > "); break;
			case ast::binary_operation::type::t_greater_equal: this->write(" >= "); break;
			case ast::binary_operation::type::t_equal: this->write(" == "); break;
			case ast::binary_operation::type::t_not_equal: this->write(" ~= "); break;
			case ast::binary_operation::type::t_and: this->write(" and "); break;
			case ast::binary_operation::type::t_or: this->write(" or "); break;
			}

			parentheses = false;

			if (expression.binary_operation_->right_operand_->type_ == ast::expression::type::t_binary_operation) {
				operand_precedence = this->get_operator_precedence(*expression.binary_operation_->right_operand_);
				if (operand_precedence == operator_precedence) {
					switch (operand_precedence) {
					case 2:
					case 4:
					case 5:
						parentheses = true;
					}
				}
				else if (operand_precedence < operator_precedence) {
					parentheses = true;
				}
			}

			this->write_expression(*expression.binary_operation_->right_operand_, parentheses);
			break;
		case ast::expression::type::t_unary_operation:
			parentheses = this->get_operator_precedence(*expression.unary_operation_->operand_) < 6;

			switch (expression.unary_operation_->type_) {
			case ast::unary_operation::type::t_minus:
				if (!parentheses
					&& expression.unary_operation_->operand_->type_ == ast::expression::type::t_unary_operation
					&& expression.unary_operation_->operand_->unary_operation_->type_ == ast::unary_operation::type::t_minus)
				{
					parentheses = true;
				}
				this->write("-");
				break;
			case ast::unary_operation::type::t_not:
				this->write("not ");
				break;
			case ast::unary_operation::type::t_length:
				this->write("#");
				break;
			}

			this->write_expression(*expression.unary_operation_->operand_, parentheses);
			break;
		}

		if (!is_potential_xhash_table_index) {
			if (use_parentheses) {
				this->write(")");
			}
			if (is_table_index) {
				this->write("]");
			}
		}
	}

	void lua::write_prefix_expression(const ast::expression& expression, const bool& is_line_start) {
		switch (expression.type_) {
		case ast::expression::type::t_variable:
			this->write_variable(*expression.variable_, is_line_start);
			break;
		case ast::expression::type::t_function_call:
			this->write_function_call(*expression.function_call_, is_line_start);
			break;
		default:
			if (is_line_start) {
				this->write(";");
			}
			this->write_expression(expression, true);
			break;
		}
	}

	void lua::write_variable(const ast::variable& variable, const bool& is_line_start) {
		switch (variable.type_) {
		case ast::variable::type::t_slot:
		case ast::variable::type::t_upvalue:
			if (!(*variable.slot_scope_)->name_.size()) {
				throw nullptr;
			}
			this->write((*variable.slot_scope_)->name_);
			break;
		case ast::variable::type::t_global:
			this->write(variable.name_);
			break;
		case ast::variable::type::t_table_index:
			this->write_prefix_expression(*variable.table_, is_line_start);
			if (variable.table_index_->type_ == ast::expression::type::t_constant && variable.table_index_->constant_->is_name_) {
				auto it = this->index_map_.find(variable.table_index_->constant_->string_);
				this->write(".", it == this->index_map_.end() ? variable.table_index_->constant_->string_ : it->second);
				break;
			}
			this->write_expression(*variable.table_index_, false, true);
			break;
		}
	}

	void lua::write_function_call(const ast::function_call& function_call, const bool& is_line_start) {
		if (function_call.is_method_) {
			this->write_prefix_expression(*function_call.function_->variable_->table_, is_line_start);
			this->write(":", function_call.function_->variable_->table_index_->constant_->string_);
		}
		else {
			this->write_prefix_expression(*function_call.function_, is_line_start);
		}
		this->write("(");
		this->write_expression_list(function_call.arguments_, function_call.multres_argument_);
		this->write(")");
	}

	void lua::write_assignment(const std::vector<ast::variable>& variables, const std::vector<ast::expression*>& expressions, const std::string& separator,
		const bool& is_line_start)
	{
		for (auto i = 0; i < variables.size(); i++) {
			this->write_variable(variables[i], i ? false : is_line_start);
			if (i != variables.size() - 1) {
				this->write(", ");
			}
		}

		if (!expressions.size()) {
			return;
		}

		this->write(separator);
		for (auto i = 0; i < expressions.size(); i++) {
			if (i != expressions.size() - 1) {
				this->write_expression(*expressions[i], false);
				this->write(", ");
				continue;
			}
			if (expressions.size() != variables.size()) {
				switch (expressions[i]->type_) {
				case ast::expression::type::t_vararg:
					if (expressions[i]->return_count_ == 1) {
						this->write_expression(*expressions[i], true);
						continue;
					}
					break;
				case ast::expression::type::t_function_call:
					if (expressions[i]->function_call_->return_count_ == 1) {
						this->write_expression(*expressions[i], true);
						continue;
					}
					break;
				default:
					break;
				}
			}
			this->write_expression(*expressions[i], false);
		}
	}

	void lua::write_expression_list(const std::vector<ast::expression*>& expressions, const ast::expression* const& multres) {
		for (auto i = 0; i < expressions.size(); i++) {
			if (i != expressions.size() - 1 || multres) {
				this->write_expression(*expressions[i], false);
				this->write(", ");
				continue;
			}
			switch (expressions[i]->type_) {
			case ast::expression::type::t_vararg:
			case ast::expression::type::t_function_call:
				this->write_expression(*expressions[i], true);
				continue;
			default:
				break;
			}
			this->write_expression(*expressions[i], false);
		}
		if (multres) {
			this->write_expression(*multres, false);
		}
	}

	void lua::write_function_definition(const ast::function& function, const bool& is_method) {
		this->write("(");

		for (auto i = is_method ? 1 : 0; i < function.parameter_names_.size(); i++) {
			this->write(function.parameter_names_[i]);
			if (i != function.parameter_names_.size() - 1 || function.is_variadic_) {
				this->write(", ");
			}
		}

		if (function.is_variadic_) {
			this->write("...");
		}

		this->write(")", NEW_LINE);
		this->indent_level_++;
		// #if defined _DEBUG
		// 	write_indent();
		// 	write("-- function ", std::to_string(function.id), NEW_LINE);
		// #endif
		if (function.block_.size()) {
			this->write_block(function, function.block_);
		}
		else {
			this->write_indent();
			this->write("return", NEW_LINE);
		}

		this->indent_level_--;
		this->write_indent();
		this->write("end");
		this->prototype_data_left_ -= function.prototype_.prototype_size_;
		// print_progress_bar(this->bc_.prototypes_total_size_ - this->prototype_data_left_, this->bc_.prototypes_total_size_);
	}

	void lua::write_number(const double& number) {
		static const auto try_string_to_number = [](const std::string& string, const double& number) -> bool {
			try {
				return std::stod(string) == number;
			}
			catch (...) {
				return false;
			}
		};

		const std::uint64_t raw_double = std::bit_cast<std::uint64_t>(number);
		if ((raw_double & DOUBLE_EXPONENT) == DOUBLE_SPECIAL) {
			this->write(raw_double & DOUBLE_SIGN ? "-1e309" : "1e309");
			return;
		}

		std::string string;
		string.resize(std::snprintf(nullptr, 0, "%1.15g", number));
		std::snprintf(string.data(), string.size() + 1, "%1.15g", number);

		if (!try_string_to_number(string, number)) {
			string.resize(std::snprintf(nullptr, 0, "%1.16g", number));
			std::snprintf(string.data(), string.size() + 1, "%1.16g", number);

			if (!try_string_to_number(string, number)) {
				string.resize(std::snprintf(nullptr, 0, "%1.17g", number));
				std::snprintf(string.data(), string.size() + 1, "%1.17g", number);
				ld_assert(try_string_to_number(string, number), "Failed to convert number to valid string", this->file_path_);
			}
		}

		this->write(string);
	}

	void lua::write_string(const std::string& string) {
		char escape_sequence[] = "\\x00";
		std::uint32_t value;
		std::uint8_t digit;
		for (auto i = 0; i < string.size(); i++) {
			value = string[i];
			if (this->unrestricted_ascii_ || !(value & 0x80)) {
				if ((string[i] >= ' ' && string[i] <= '~') || (this->unrestricted_ascii_ && (string[i] & 0x80) != 0)) {
					switch (string[i]) {
					case '"':
					case '\\':
						this->write_buffer_ += '\\';
					}
					this->write_buffer_ += string[i];
					continue;
				}
				switch (string[i]) {
				case '\a': this->write("\\a"); continue;
				case '\b': this->write("\\b"); continue;
				case '\t': this->write("\\t"); continue;
				case '\n': this->write("\\n"); continue;
				case '\v': this->write("\\v"); continue;
				case '\f': this->write("\\f"); continue;
				case '\r': this->write("\\r"); continue;
				}
			}
			else if ((value & 0xE0) == 0xC0) {
				if (i + 1 < string.size()) {
					value <<= 8;
					value |= string[i + 1];
					if ((value & 0xC0) == 0x80 && value >= 0xC2A0 && value <= 0xDFBF) {
						this->write_buffer_ += string[i];
						this->write_buffer_ += string[i + 1];
						i++;
						continue;
					}
				}
			}
			else if ((value & 0xF0) == 0xE0) {
				if (i + 2 < string.size()) {
					value <<= 16;
					value |= VAL_AS(std::uint16_t, string[i + 1] << 0x08);
					value |= string[i + 2];
					if ((value & 0xC0C0) == 0x8080 && ((value >= 0xE0A080 && value < 0xEDA080) || (value > 0xEDBFBF && value <= 0xEFBFBF))) {
						this->write_buffer_ += string[i];
						this->write_buffer_ += string[i + 1];
						this->write_buffer_ += string[i + 2];
						i += 2;
						continue;
					}
				}
			}
			else if ((value & 0xF8) == 0xF0) {
				if (i + 3 < string.size()) {
					value <<= 24;
					value |= VAL_AS(std::uint32_t, string[i + 1] << 0x10);
					value |= VAL_AS(std::uint16_t, string[i + 2] << 0x08);
					value |= string[i + 3];

					if ((value & 0xC0C0C0) == 0x808080 && value >= 0xF0908080 && value <= 0xF48FBFBF) {
						this->write_buffer_ += string[i];
						this->write_buffer_ += string[i + 1];
						this->write_buffer_ += string[i + 2];
						this->write_buffer_ += string[i + 3];
						i += 3;
						continue;
					}
				}
			}

			for (auto j = 2; j--;) {
				digit = (string[i] >> j * 4) & 0xF;
				escape_sequence[3 - j] = digit >= 0xA ? 'A' + digit - 0xA : '0' + digit;
			}

			this->write_buffer_ += escape_sequence;
		}
	}

	std::uint8_t lua::get_operator_precedence(const ast::expression& expression) {
		switch (expression.type_) {
		case ast::expression::type::t_binary_operation:
			switch (expression.binary_operation_->type_) {
			case ast::binary_operation::type::t_exponentiation:
				return 7;
			case ast::binary_operation::type::t_multiplication:
			case ast::binary_operation::type::t_division:
			case ast::binary_operation::type::t_modulo:
				return 5;
			case ast::binary_operation::type::t_addition:
			case ast::binary_operation::type::t_subtraction:
				return 4;
			case ast::binary_operation::type::t_concatenation:
				return 3;
			case ast::binary_operation::type::t_less_than:
			case ast::binary_operation::type::t_less_equal:
			case ast::binary_operation::type::t_greater_than:
			case ast::binary_operation::type::t_greater_equal:
			case ast::binary_operation::type::t_equal:
			case ast::binary_operation::type::t_not_equal:
				return 2;
			case ast::binary_operation::type::t_and:
				return 1;
			case ast::binary_operation::type::t_or:
				return 0;
			}
		case ast::expression::type::t_unary_operation:
			return 6;
		default:
			return 8;
		}
	}

	void lua::write(const std::string& string) {
		this->write_buffer_ += string;
	}

	void lua::write_indent() {
		return this->write(std::string(this->indent_level_, '\t'));
	}

	void lua::create_file() {
		this->file_ = CreateFileA(this->file_path_.c_str(), GENERIC_WRITE, NULL, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	}

	void lua::close_file() {
		if (this->file_ == INVALID_HANDLE_VALUE) {
			return;
		}
		CloseHandle(this->file_);
		this->file_ = INVALID_HANDLE_VALUE;
	}

	void lua::write_file() {
		DWORD chars_written = 0;
		ld_assert(WriteFile(this->file_, this->write_buffer_.data(), this->write_buffer_.size(), &chars_written, NULL)
			&& !(this->write_buffer_.size() - chars_written), "Failed writing to file", this->file_path_);
		this->write_buffer_.clear();
		this->write_buffer_.shrink_to_fit();
	}
}

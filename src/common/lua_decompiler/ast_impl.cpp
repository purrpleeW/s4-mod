#include "common_core.hpp"
#include "lua_decompiler/ast_impl.hpp"

#include "logger/logger.hpp"
#include "lua_decompiler/ld_common.hpp"
#include "lua_decompiler/ast/binary_operation.hpp"
#include "lua_decompiler/ast/condition_builder.hpp"
#include "lua_decompiler/ast/constant.hpp"
#include "lua_decompiler/ast/function.hpp"
#include "lua_decompiler/ast/function_call.hpp"
#include "lua_decompiler/ast/local.hpp"
#include "lua_decompiler/ast/statement.hpp"
#include "lua_decompiler/ast/table.hpp"
#include "lua_decompiler/ast/unary_operation.hpp"
#include "lua_decompiler/bytecode/constant.hpp"
#include "lua_decompiler/bytecode/number_constant.hpp"
#include "lua_decompiler/bytecode/prototype.hpp"

namespace lua_decompiler {
	ast::ast(const bytecode& bc, const bool& ignore_debug_info, const bool& minimize_diffs)
		: bc_(bc)
		, ignore_debug_info_(ignore_debug_info)
		, minimize_diffs_(minimize_diffs) 
	{}

	ast::~ast() {
		for (auto x = this->statements_.size(); x--;) {
			delete this->statements_[x];
		}
		for (auto x = this->functions_.size(); x--;) {
			delete this->functions_[x];
		}
		for (auto x = this->expressions_.size(); x--;) {
			delete this->expressions_[x];
		}
	}

	std::uint32_t ast::get_block_index_from_id(const std::vector<statement*>& block, const std::uint32_t& id) {
		for (auto x = block.size(); x-- && (block[x]->instruction_.id_ == INVALID_ID || block[x]->instruction_.id_ >= id);) {
			if (block[x]->instruction_.id_ != id) {
				continue;
			}
			return x;
		}
		return INVALID_ID;
	}

	std::uint32_t ast::get_extended_id_from_statement(statement* const& statement_in) {
		switch (statement_in->type_) {
		case statement::type::t_goto:
		case statement::type::t_break:
			if (statement_in->instruction_.type_ == bytecode::opcode::op_jmp) {
				return statement_in->instruction_.target_;
			}
		default:
			break;
		}
		return statement_in->instruction_.id_;
	}

	std::uint32_t ast::get_label_from_next_statement(function& function_in, const block_info& block_info_in, const bool& return_extended_label,
		const bool& exclude_declaration)
	{
		if (block_info_in.index_ == block_info_in.block_.size() - 1) {
			return block_info_in.previous_block_
				? get_label_from_next_statement(function_in, *block_info_in.previous_block_, return_extended_label, false)
				: INVALID_ID;
		}

		statement* s = block_info_in.block_[block_info_in.index_ + 1];
		if (exclude_declaration && s->type_ == statement::type::t_declaration) {
			if (s->block_.size()) {
				s = s->block_.front();
			}
			else if (block_info_in.index_ + 2 != block_info_in.block_.size()) {
				s = block_info_in.block_[block_info_in.index_ + 2];
			}
			else {
				return block_info_in.previous_block_
					? get_label_from_next_statement(function_in, *block_info_in.previous_block_, return_extended_label, false)
					: INVALID_ID;
			}
		}

		switch (s->type_) {
		case statement::type::t_empty:
		case statement::type::t_goto:
		case statement::type::t_break:
			if (return_extended_label && s->instruction_.type_ == bytecode::opcode::op_jmp) {
				return function_in.get_label_from_id(s->instruction_.target_);
			}
		default:
			break;
		}
		return s->instruction_.label_;
	}

	bool ast::is_valid_block(function& function_in, const block_info& block_info_in, const std::uint32_t& block_begin) {
		if (block_info_in.index_ == block_info_in.block_.size() - 1) {
			return block_info_in.previous_block_ ? is_valid_block(function_in, *block_info_in.previous_block_, block_begin) : true;
		}
		const std::uint32_t block_end = block_info_in.block_[block_info_in.index_ + 1]->instruction_.label_ != INVALID_ID
			? function_in.labels_[block_info_in.block_[block_info_in.index_ + 1]->instruction_.label_].target_
			: block_info_in.block_[block_info_in.index_ + 1]->instruction_.id_;
		return block_end == INVALID_ID || block_end <= block_begin || function_in.is_valid_block_range(block_begin, block_end - 1, false);
	}

	void ast::check_valid_name(constant* const& constant_in) {
		static const std::string keywords[] = {
			"and", "break", "do", "else", "elseif", "end", "false",
			"for", "function", "if", "in", "local", "nil", "not",
			"or", "repeat", "return", "then", "true", "until", "while"
		};

		if (!constant_in->string_.size() || constant_in->string_.front() < 'A') {
			return;
		}

		for (auto x = constant_in->string_.size(); x--;) {
			if (constant_in->string_[x] < '0') {
				return;
			}
			switch (constant_in->string_[x]) {
			case ':':
			case ';':
			case '<':
			case '=':
			case '>':
			case '?':
			case '@':
			case '[':
			case '\\':
			case ']':
			case '^':
			case '`':
			case '{':
			case '|':
			case '}':
			case '~':
			case '\x7F':
				return;
			}
		}

		for (auto& keyword : keywords) {
			if (constant_in->string_ == keyword) {
				return;
			}
		}

		constant_in->is_name_ = true;
	}

	ast::constant_type ast::get_constant_type(expression* const& expression_in) {
		static const auto is_valid_number_constant = [](const double& number)->bool {
			const std::uint64_t raw_double = std::bit_cast<std::uint64_t>(number);
			return (raw_double & DOUBLE_EXPONENT) == DOUBLE_SPECIAL ? !(raw_double & DOUBLE_FRACTION) : raw_double != DOUBLE_NEGATIVE_ZERO;
		};

		switch (expression_in->type_) {
		case expression::type::t_constant:
			switch (expression_in->constant_->type_) {
			case constant::type::t_nil:
				return constant_type::t_nil;
			case constant::type::t_false:
			case constant::type::t_true:
			case constant::type::t_xhash:
			case constant::type::t_string:
				return constant_type::t_bool;
			case constant::type::t_number:
				return constant_type::t_number;
			default:
				break;
			}
			break;
		case expression::type::t_binary_operation:
			switch (expression_in->binary_operation_->type_) {
			case binary_operation::type::t_addition:
			case binary_operation::type::t_subtraction:
			case binary_operation::type::t_multiplication:
			case binary_operation::type::t_division:
			case binary_operation::type::t_exponentiation:
			case binary_operation::type::t_modulo: {
				const auto l = expression_in->binary_operation_->left_operand_;
				const auto r = expression_in->binary_operation_->right_operand_;
				if (get_constant_type(l) != constant_type::t_number || get_constant_type(r) != constant_type::t_number) {
					break;
				}
				double number;
				switch (expression_in->binary_operation_->type_) {
				case binary_operation::type::t_addition:
					number = l->constant_->number_ + r->constant_->number_;
					break;
				case binary_operation::type::t_subtraction:
					number = l->constant_->number_ - r->constant_->number_;
					break;
				case binary_operation::type::t_multiplication:
					number = l->constant_->number_ * r->constant_->number_;
					break;
				case binary_operation::type::t_division:
					number = l->constant_->number_ / r->constant_->number_;
					break;
				case binary_operation::type::t_exponentiation:
					number = std::pow(l->constant_->number_, r->constant_->number_);
					break;
				case binary_operation::type::t_modulo:
					number = std::fmod(l->constant_->number_, r->constant_->number_);
					break;
				default:
					break;
				}
				if (is_valid_number_constant(number)) {
					return constant_type::t_number;
				}
			}
			default:
				break;
			}
			break;
		case expression::type::t_unary_operation:
			switch (expression_in->unary_operation_->type_) {
			case unary_operation::type::t_not:
				if (get_constant_type(expression_in->unary_operation_->operand_) != constant_type::t_invalid) {
					return constant_type::t_bool;
				}
				break;
			case unary_operation::type::t_minus:
				if (expression_in->unary_operation_->operand_->type_ == expression::type::t_constant) {
					switch (expression_in->unary_operation_->operand_->constant_->type_) {
					case constant::type::t_number:
						if (!is_valid_number_constant(-expression_in->unary_operation_->operand_->constant_->number_)) {
							break;
						}
					case constant::type::t_cdata_signed:
					case constant::type::t_cdata_unsigned:
					case constant::type::t_cdata_imaginary:
						return constant_type::t_number;
					default:
						break;
					}
				}
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		return constant_type::t_invalid;
	}

	void ast::operator()() {
		this->chunk_ = this->new_function(*this->bc_.main_, 0);
		this->is_fr2_enabled_ = this->bc_.header_.version_ != bytecode::version_01 && (this->bc_.header_.flags_ & bytecode::f_fr2);
		this->prototype_data_left_ = this->bc_.prototypes_total_size_;
		std::uint32_t function_counter = 0;
		this->build_functions(*this->chunk_, function_counter);
		this->functions_.shrink_to_fit();
		this->statements_.shrink_to_fit();
		this->expressions_.shrink_to_fit();
	}

	ast::function*& ast::new_function(const bytecode::prototype& prototype, const std::uint32_t& level) {
		return this->functions_.emplace_back(new function(prototype, level, this->ignore_debug_info_));
	}

	ast::statement*& ast::new_statement(const int& type) {
		return this->statements_.emplace_back(new statement(VAL_AS(statement::type, type)));
	}

	ast::expression*& ast::new_expression(const int& type) {
		return this->expressions_.emplace_back(new expression(VAL_AS(expression::type, type)));
	}

	void ast::build_functions(function& function_in, std::uint32_t& function_counter) {
		function_in.id_ = function_counter;
		function_counter++;
		this->build_instructions(function_in);
		function_in.used_globals_.shrink_to_fit();
		if (!function_in.has_debug_info_) {
			function_in.slot_scope_collector_.build_upvalue_scopes();
		}
		this->build_slot_scopes(function_in, function_in.block_, nullptr);
		ld_assert(function_in.slot_scope_collector_.assert_scopes_closed(), "Failed to close slot scopes", this->bc_.file_path_);
		this->eliminate_slots(function_in, function_in.block_, nullptr);
		this->eliminate_conditions(function_in, function_in.block_, nullptr);
		this->build_if_statements(function_in, function_in.block_, nullptr);
		this->clean_up(function_in);
		function_in.block_.shrink_to_fit();
		this->prototype_data_left_ -= function_in.prototype_.prototype_size_;
		// print_progress_bar(this->bc_.prototypesTotalSize - prototypeDataLeft, bytecode.prototypesTotalSize);
		for (auto x = function_in.child_functions_.size(); x--;) {
			this->build_functions(*function_in.child_functions_[x], function_counter);
		}
	}

	void ast::build_instructions(function& function_in) {
		std::vector<std::uint8_t> upvalues;
		function_in.block_.resize(function_in.prototype_.instructions_.size(), nullptr);

		for (auto x = function_in.block_.size(); x--;) {
			function_in.block_[x] = this->new_statement(ENUM_UNDER(statement::type::t_instruction));
			function_in.block_[x]->instruction_.type_ = function_in.prototype_.instructions_[x].type_;
			function_in.block_[x]->instruction_.a_ = function_in.prototype_.instructions_[x].a_;
			function_in.block_[x]->instruction_.b_ = function_in.prototype_.instructions_[x].b_;
			function_in.block_[x]->instruction_.c_ = function_in.prototype_.instructions_[x].c_;
			function_in.block_[x]->instruction_.d_ = function_in.prototype_.instructions_[x].d_;
			function_in.block_[x]->instruction_.id_ = x;

			switch (function_in.block_[x]->instruction_.type_) {
			case bytecode::opcode::op_fnew:
				function_in.block_[x]->function_ = this->new_function(*function_in.get_constant(function_in.block_[x]->instruction_.d_).prototype_,
					function_in.level_ + 1);
				function_in.child_functions_.emplace_back(function_in.block_[x]->function_);
				function_in.block_[x]->function_->upvalues_.resize(function_in.block_[x]->function_->prototype_.upvalues_.size());

				for (auto y = function_in.block_[x]->function_->upvalues_.size(); y--;) {
					function_in.block_[x]->function_->upvalues_[y].slot_ = function_in.block_[x]->function_->prototype_.upvalues_[y];

					if (!(function_in.block_[x]->function_->prototype_.upvalues_[y] & bytecode::uv_local)) {
						function_in.block_[x]->function_->upvalues_[y].slot_scope_
							= function_in.upvalues_[function_in.block_[x]->function_->upvalues_[y].slot_].slot_scope_;
						continue;
					}

					function_in.block_[x]->function_->upvalues_[y].local_ = true;
					if (function_in.block_[x]->function_->upvalues_[y].slot_ >= function_in.prototype_.header_.parameters_) {
						upvalues.emplace_back(function_in.block_[x]->function_->upvalues_[y].slot_);
					}
				}

				if (!upvalues.empty()) {
					function_in.slot_scope_collector_.add_upvalues(function_in.block_[x]->instruction_.id_, upvalues);
					upvalues.clear();
				}
				continue;
			case bytecode::opcode::op_callmt:
			case bytecode::opcode::op_callt:
			case bytecode::opcode::op_retm:
			case bytecode::opcode::op_ret:
			case bytecode::opcode::op_ret0:
			case bytecode::opcode::op_ret1:
				function_in.block_[x]->type_ = statement::type::t_return;
				continue;
			case bytecode::opcode::op_uclo:
			case bytecode::opcode::op_isnext:
			case bytecode::opcode::op_fori:
			case bytecode::opcode::op_forl:
			case bytecode::opcode::op_iterl:
			case bytecode::opcode::op_loop:
			case bytecode::opcode::op_jmp:
				function_in.block_[x]->instruction_.target_ = function_in.block_[x]->instruction_.id_
					+ (function_in.block_[x]->instruction_.d_ - bytecode::op_jmp_bias + 1);
				continue;
			default:
				break;
			}
		}
		function_in.child_functions_.shrink_to_fit();
		return this->assign_debug_info(function_in);
	}

	void ast::assign_debug_info(function& function_in) {
		if (!function_in.has_debug_info_) {
			return this->group_jumps(function_in);
		}

		std::vector<std::uint32_t> active_local_scopes;
		function_in.parameter_names_.resize(function_in.prototype_.header_.parameters_);
		for (auto x = function_in.parameter_names_.size(); x--;) {
			function_in.parameter_names_[x] = function_in.prototype_.variable_infos_[x].name_;
			active_local_scopes.emplace_back(function_in.prototype_.variable_infos_[x].scope_end_);
		}

		std::uint32_t index;
		for (auto x = function_in.parameter_names_.size(); x < function_in.prototype_.variable_infos_.size(); x++) {
			ld_assert(!active_local_scopes.size()
				|| function_in.prototype_.variable_infos_[x].scope_begin_ >= active_local_scopes.back()
				|| function_in.prototype_.variable_infos_[x].scope_end_ <= active_local_scopes.back(),
				"Illegal variable scope border overlap", this->bc_.file_path_);

			while (active_local_scopes.size() && function_in.prototype_.variable_infos_[x].scope_end_ > active_local_scopes.back()) {
				active_local_scopes.pop_back();
			}

			if (function_in.prototype_.variable_infos_[x].type_ != bytecode::variable_info::type::t_str) {
				active_local_scopes.emplace_back(function_in.prototype_.variable_infos_[x].scope_end_);
				continue;
			}

			if (function_in.locals_.size()
				&& function_in.prototype_.variable_infos_[x].scope_begin_ == function_in.prototype_.variable_infos_[x].scope_end_
				&& function_in.locals_.back().scope_end_ == function_in.prototype_.variable_infos_[x].scope_end_)
			{
				index = this->get_block_index_from_id(function_in.block_, function_in.prototype_.variable_infos_[x].scope_begin_);
				switch (function_in.block_[index]->instruction_.type_) {
				case bytecode::opcode::op_kpri:
					if (function_in.block_[index]->instruction_.d_) {
						break;
					}
				case bytecode::opcode::op_knil:
					if ((function_in.block_[index]->instruction_.type_ == bytecode::opcode::op_kpri
							? function_in.block_[index]->instruction_.a_
							: function_in.block_[index]->instruction_.d_)
						< active_local_scopes.size())
					{
						while (active_local_scopes.size() != function_in.locals_.back().base_slot_) {
							ld_assert(active_local_scopes.size() && active_local_scopes.back() == function_in.prototype_.variable_infos_[x].scope_end_,
								"Unable to build variable scope", this->bc_.file_path_);
							active_local_scopes.pop_back();
						}

						function_in.locals_.emplace_back();
						function_in.locals_.back().base_slot_ = active_local_scopes.size();
						function_in.locals_.back().scope_begin_ = function_in.prototype_.variable_infos_[x].scope_begin_;
						function_in.locals_.back().scope_end_ = function_in.prototype_.variable_infos_[x].scope_end_;
						function_in.locals_.back().exclude_block_ = function_in.locals_[function_in.locals_.size() - 2].scope_begin_
							== function_in.locals_[function_in.locals_.size() - 2].scope_end_
								? function_in.locals_[function_in.locals_.size() - 2].exclude_block_
								: true;
						function_in.add_jump(function_in.locals_.back().scope_begin_, function_in.locals_.back().scope_end_ + 1);
					}
				default:
					break;
				}
			}

			if (!function_in.locals_.size()
				|| function_in.prototype_.variable_infos_[x].scope_begin_ != function_in.locals_.back().scope_begin_
				|| function_in.prototype_.variable_infos_[x].scope_end_ != function_in.locals_.back().scope_end_)
			{
				function_in.locals_.emplace_back();
				function_in.locals_.back().base_slot_ = active_local_scopes.size();
				function_in.locals_.back().scope_begin_ = function_in.prototype_.variable_infos_[x].scope_begin_;
				function_in.locals_.back().scope_end_ = function_in.prototype_.variable_infos_[x].scope_end_;
				function_in.add_jump(function_in.locals_.back().scope_begin_, function_in.locals_.back().scope_end_ + 1);
			}

			function_in.locals_.back().names_.emplace_back(function_in.prototype_.variable_infos_[x].name_);
			active_local_scopes.emplace_back(function_in.locals_.back().scope_end_);
		}

		for (auto x = function_in.locals_.size(); x--;) {
			function_in.locals_[x].names_.shrink_to_fit();
		}

		function_in.locals_.shrink_to_fit();
		return this->group_jumps(function_in);
	}

	void ast::group_jumps(function& function_in) {
		for (auto x = function_in.block_.size(); x--;) {
			switch (function_in.block_[x]->instruction_.type_) {
			case bytecode::opcode::op_istc:
			case bytecode::opcode::op_isfc:
				function_in.add_jump(function_in.block_[x]->instruction_.id_, function_in.block_[x]->instruction_.id_ + 2);
			case bytecode::opcode::op_islt:
			case bytecode::opcode::op_isge:
			case bytecode::opcode::op_isle:
			case bytecode::opcode::op_isgt:
			case bytecode::opcode::op_iseqv:
			case bytecode::opcode::op_isnev:
			case bytecode::opcode::op_iseqs:
			case bytecode::opcode::op_isnes:
			case bytecode::opcode::op_iseqn:
			case bytecode::opcode::op_isnen:
			case bytecode::opcode::op_iseqp:
			case bytecode::opcode::op_isnep:
			case bytecode::opcode::op_ist:
			case bytecode::opcode::op_isf:
				function_in.block_[x]->type_ = statement::type::t_condition;
				function_in.block_[x]->instruction_.target_ = function_in.block_[x + 1]->instruction_.target_;
				function_in.block_.erase(function_in.block_.begin() + x + 1);
				function_in.slot_scope_collector_.add_jump(function_in.block_[x]->instruction_.id_ + 1, function_in.block_[x]->instruction_.target_);
				continue;
			case bytecode::opcode::op_uclo:
				function_in.slot_scope_collector_.add_upvalue_close(function_in.block_[x]->instruction_.id_, function_in.block_[x]->instruction_.target_,
					function_in.block_[x]->instruction_.a_);
			case bytecode::opcode::op_jmp:
				function_in.block_[x]->type_ = statement::type::t_goto;
			case bytecode::opcode::op_loop:
				function_in.add_jump(function_in.block_[x]->instruction_.id_, function_in.block_[x]->instruction_.target_);
			default:
				continue;
			}
		}

		function_in.labels_.shrink_to_fit();
		std::uint32_t index;
		for (auto x = function_in.block_.size(); x--;) {
			function_in.block_[x]->instruction_.label_ = function_in.get_label_from_id(function_in.block_[x]->instruction_.id_);
			switch (function_in.block_[x]->instruction_.type_) {
			case bytecode::opcode::op_uclo:
				if (function_in.block_[x]->instruction_.d_ == bytecode::op_jmp_bias
					|| function_in.block_[x]->instruction_.target_ == this->get_extended_id_from_statement(function_in.block_[x + 1]))
				{
					function_in.block_[x]->type_ = statement::type::t_empty;
					function_in.remove_jump(function_in.block_[x]->instruction_.id_, function_in.block_[x]->instruction_.target_);
				}
				continue;
			case bytecode::opcode::op_iterc:
				index = this->get_block_index_from_id(function_in.block_, function_in.labels_[function_in.block_[x]->instruction_.label_].jump_ids_.front());
				function_in.block_[index]->type_ = statement::type::t_instruction;
				function_in.remove_jump(function_in.block_[index]->instruction_.id_, function_in.block_[index]->instruction_.target_);
				continue;
			case bytecode::opcode::op_jmp:
				if (function_in.block_[x]->type_ != statement::type::t_goto) {
					continue;
				}
				function_in.slot_scope_collector_.add_jump(function_in.block_[x]->instruction_.id_, function_in.block_[x]->instruction_.target_);
				if (function_in.block_[x]->instruction_.target_ == function_in.block_[x]->instruction_.id_
					|| !x
					|| function_in.block_[x - 1]->instruction_.type_ != bytecode::opcode::op_jmp
					|| function_in.block_[x - 1]->instruction_.d_ != bytecode::op_jmp_bias
					|| function_in.labels_[function_in.block_[x]->instruction_.label_].jump_ids_.size() > 1)
				{
					continue;
				}
				function_in.remove_jump(function_in.block_[x - 1]->instruction_.id_, function_in.block_[x - 1]->instruction_.target_);
				function_in.block_[x - 1]->type_ = statement::type::t_condition;
				function_in.block_[x - 1]->instruction_.target_ = function_in.block_[x]->instruction_.target_;
				function_in.block_.erase(function_in.block_.begin() + x);
			default:
				continue;
			}
		}

		for (auto x = function_in.locals_.size(); x--;) {
			function_in.remove_jump(function_in.locals_[x].scope_begin_, function_in.locals_[x].scope_end_ + 1);
		}

		for (auto x = function_in.block_.size(); x--;) {
			if (x
				&& function_in.block_[x]->type_ == statement::type::t_return
				&& function_in.block_[x - 1]->type_ == statement::type::t_return
				&& function_in.is_valid_label(function_in.block_[x]->instruction_.label_)
				&& function_in.labels_[function_in.block_[x]->instruction_.label_].jump_ids_.size() == 1)
			{
				index = this->get_block_index_from_id(function_in.block_, function_in.labels_[function_in.block_[x]->instruction_.label_].jump_ids_.back());
				if (index != INVALID_ID
					&& function_in.block_[index]->type_ == statement::type::t_goto
					&& function_in.block_[index]->instruction_.type_ == bytecode::opcode::op_uclo)
				{
					function_in.remove_jump(function_in.block_[index]->instruction_.id_, function_in.block_[index]->instruction_.target_);
					function_in.block_[index]->type_ = statement::type::t_return;
					function_in.block_[index]->instruction_.type_ = function_in.block_[x]->instruction_.type_;
					function_in.block_[index]->instruction_.a_ = function_in.block_[x]->instruction_.a_;
					function_in.block_[index]->instruction_.b_ = function_in.block_[x]->instruction_.b_;
					function_in.block_[index]->instruction_.c_ = function_in.block_[x]->instruction_.c_;
					function_in.block_[index]->instruction_.d_ = function_in.block_[x]->instruction_.d_;
					function_in.block_[x]->type_ = statement::type::t_empty;
					continue;
				}
			}
			if (function_in.block_[x]->instruction_.type_ == bytecode::opcode::op_ret0) {
				function_in.block_[x]->type_ = statement::type::t_empty;
			}
			break;
		}

		return this->build_loops(function_in);
	}

	void ast::build_loops(function& function_in) {
		static const auto build_break_statements = [](std::vector<statement*>& block, const std::uint32_t& break_target)->void {
			for (auto x = block.size(); x--;) {
				if (block[x]->type_ != statement::type::t_goto || block[x]->instruction_.target_ != break_target) {
					continue;
				}
				block[x]->type_ = statement::type::t_break;
			}
		};

		std::uint32_t target_index, break_target;
		for (auto i = function_in.block_.size(); i--;) {
			if (function_in.block_[i]->type_ != statement::type::t_instruction) {
				continue;
			}
			switch (function_in.block_[i]->instruction_.type_) {
			case bytecode::opcode::op_isnext:
			case bytecode::opcode::op_jmp:
				function_in.block_[i]->type_ = statement::type::t_generic_for;
				target_index = this->get_block_index_from_id(function_in.block_, function_in.block_[i]->instruction_.target_);
				break_target = this->get_extended_id_from_statement(function_in.block_[target_index + 2]);
				function_in.block_[target_index]->instruction_.target_ = function_in.block_[i]->instruction_.label_;
				function_in.block_[i]->instruction_ = function_in.block_[target_index]->instruction_;
				function_in.block_[i]->instruction_.id_ = function_in.block_[target_index + 1]->instruction_.target_ - 1;
				function_in.block_[i]->instruction_.label_ = function_in.block_[i]->instruction_.target_;
				function_in.block_[i]->instruction_.target_ = function_in.block_[target_index + 1]->instruction_.id_ + 1;
				function_in.block_[target_index]->type_ = statement::type::t_empty;
				function_in.block_[i]->block_.reserve(target_index - i);
				function_in.block_[i]->block_.insert(function_in.block_[i]->block_.begin(), function_in.block_.begin() + i + 1,
					function_in.block_.begin() + target_index + 1);
				function_in.block_.erase(function_in.block_.begin() + i + 1, function_in.block_.begin() + target_index + 2);
				function_in.slot_scope_collector_.add_loop(function_in.block_[i]->instruction_.id_, function_in.block_[i]->instruction_.target_);
				build_break_statements(function_in.block_[i]->block_, break_target);
				this->build_local_scopes(function_in, function_in.block_[i]->block_);
				continue;
			case bytecode::opcode::op_fori:
				function_in.block_[i]->type_ = statement::type::t_numeric_for;
				target_index = this->get_block_index_from_id(function_in.block_, function_in.block_[i]->instruction_.target_);
				break_target = this->get_extended_id_from_statement(function_in.block_[target_index]);
				function_in.block_[target_index - 1]->type_ = statement::type::t_empty;
				function_in.block_[i]->block_.reserve(target_index - 1 - i);
				function_in.block_[i]->block_.insert(function_in.block_[i]->block_.begin(), function_in.block_.begin() + i + 1,
					function_in.block_.begin() + target_index);
				function_in.block_.erase(function_in.block_.begin() + i + 1, function_in.block_.begin() + target_index);
				function_in.slot_scope_collector_.add_loop(function_in.block_[i]->instruction_.id_, function_in.block_[i]->instruction_.target_);
				build_break_statements(function_in.block_[i]->block_, break_target);
				this->build_local_scopes(function_in, function_in.block_[i]->block_);
				continue;
			case bytecode::opcode::op_loop:
				ld_assert(function_in.block_[i]->instruction_.target_ >= function_in.block_[i]->instruction_.id_, "LOOP instruction has invalid jump target",
					this->bc_.file_path_);
				function_in.remove_jump(function_in.block_[i]->instruction_.id_, function_in.block_[i]->instruction_.target_);

				if (function_in.block_[i]->instruction_.target_ == function_in.block_[i]->instruction_.id_) {
					ld_assert(i + 1 < function_in.block_.size()
						&& function_in.block_[i + 1]->type_ == statement::type::t_goto
						&& function_in.block_[i + 1]->instruction_.target_ <= function_in.block_[i]->instruction_.id_
						&& !function_in.is_valid_label(function_in.block_[i + 1]->instruction_.label_),
						"Invalid goto loop", this->bc_.file_path_);
					function_in.block_[i]->type_ = statement::type::t_empty;
					function_in.block_[i + 1]->instruction_.type_ = function_in.block_[i]->instruction_.type_;
					continue;
				}

				function_in.block_[i]->type_ = statement::type::t_loop;
				target_index = this->get_block_index_from_id(function_in.block_, function_in.block_[i]->instruction_.target_);
				break_target = this->get_extended_id_from_statement(function_in.block_[target_index]);
				function_in.block_[i]->block_.reserve(target_index - 1 - i);
				function_in.block_[i]->block_.insert(function_in.block_[i]->block_.begin(), function_in.block_.begin() + i + 1,
					function_in.block_.begin() + target_index);
				function_in.block_.erase(function_in.block_.begin() + i + 1, function_in.block_.begin() + target_index);
				function_in.slot_scope_collector_.add_loop(function_in.block_[i]->instruction_.id_, function_in.block_[i]->instruction_.target_);
				build_break_statements(function_in.block_[i]->block_, break_target);

				if (function_in.block_[i]->block_.size()
					//TODO
					&& function_in.block_[i]->block_.back()->type_ == statement::type::t_condition
					&& function_in.is_valid_label(function_in.block_[i]->instruction_.label_)
					&& break_target != function_in.block_[i]->instruction_.id_)
				{
					for (auto j = function_in.labels_[function_in.block_[i]->instruction_.label_].jump_ids_.size(); j--
						&& function_in.labels_[function_in.block_[i]->instruction_.label_].jump_ids_[j] > function_in.block_[i]->instruction_.id_;)
					{
						if (function_in.labels_[function_in.block_[i]->instruction_.label_].jump_ids_[j] >= function_in.block_[i]->instruction_.target_) {
							continue;
						}
						target_index = this->get_block_index_from_id(function_in.block_[i]->block_,
							function_in.labels_[function_in.block_[i]->instruction_.label_].jump_ids_[j] - 1);
						if (target_index != INVALID_ID && function_in.block_[i]->block_[target_index]->type_ == statement::type::t_condition) {
							function_in.block_[i]->block_.emplace_back(this->new_statement(ENUM_UNDER(statement::type::t_break)));
							function_in.block_[i]->block_.back()->instruction_.type_ = bytecode::opcode::op_jmp;
							function_in.block_[i]->block_.back()->instruction_.target_ = break_target;
							function_in.block_[i]->block_.emplace_back(this->new_statement(ENUM_UNDER(statement::type::t_goto)));
							function_in.block_[i]->block_.back()->instruction_.type_ = bytecode::opcode::op_jmp;
							function_in.block_[i]->block_.back()->instruction_.target_ = function_in.block_[i]->instruction_.id_;
						}
						break;
					}
				}
				this->build_local_scopes(function_in, function_in.block_[i]->block_);
			default:
				continue;
			}
		}
		function_in.slot_scope_collector_.upvalue_infos_.shrink_to_fit();
		return this->build_local_scopes(function_in, function_in.block_);
	}

	void ast::build_local_scopes(function& function_in, std::vector<statement*>& block) {
		if (!function_in.has_debug_info_) {
			return this->build_expressions(function_in, block);
		}
		std::uint32_t scope_begin_index, scope_end_index;
		for (auto x = function_in.locals_.size(); x--;) {
			scope_begin_index = this->get_block_index_from_id(block, function_in.locals_[x].scope_begin_);
			if (scope_begin_index == INVALID_ID) {
				continue;
			}
			switch (block[scope_begin_index]->type_) {
			case statement::type::t_numeric_for:
			case statement::type::t_generic_for:
				block[scope_begin_index]->locals_ = &function_in.locals_[x];
			default:
				continue;
			}

			scope_begin_index++;
			block.emplace(block.begin() + scope_begin_index, this->new_statement(ENUM_UNDER(statement::type::t_declaration)));
			block[scope_begin_index]->locals_ = &function_in.locals_[x];

			if (function_in.locals_[x].scope_end_ > function_in.locals_[x].scope_begin_) {
				block[scope_begin_index]->instruction_.id_ = function_in.locals_[x].scope_begin_ + 1;
				scope_end_index = this->get_block_index_from_id(block, function_in.locals_[x].scope_end_ + 1);
				if (scope_end_index == INVALID_ID) {
					scope_end_index = block.size();
				}

				while (block[scope_end_index - 1]->type_ == statement::type::t_declaration && block[scope_end_index - 1]->locals_->exclude_block_) {
					scope_end_index--;
				}

				block[scope_begin_index]->block_.reserve(scope_end_index - 1 - scope_begin_index);
				block[scope_begin_index]->block_.insert(block[scope_begin_index]->block_.begin(), block.begin() + scope_begin_index + 1,
					block.begin() + scope_end_index);
				block.erase(block.begin() + scope_begin_index + 1, block.begin() + scope_end_index);
				this->build_expressions(function_in, block[scope_begin_index]->block_);
			}
		}
		return this->build_expressions(function_in, block);
	}

	void ast::build_expressions(function& function_in, std::vector<statement*>& block) {
		for (auto i = block.size(); i--;) {
			switch (block[i]->type_) {
			case statement::type::t_instruction:
				block[i]->type_ = statement::type::t_assignment;
				block[i]->assignment_.expressions_.resize(1, nullptr);

				switch (block[i]->instruction_.type_) {
				case bytecode::opcode::op_mov:
					block[i]->assignment_.expressions_.back() = this->new_slot(block[i]->instruction_.d_);
					block[i]->assignment_.register_slots(block[i]->assignment_.expressions_.back());
					break;
				case bytecode::opcode::op_not:
				case bytecode::opcode::op_unm:
				case bytecode::opcode::op_len:
					block[i]->assignment_.expressions_.back() = this->new_expression(ENUM_UNDER(expression::type::t_unary_operation));
					switch (block[i]->instruction_.type_) {
					case bytecode::opcode::op_not:
						block[i]->assignment_.expressions_.back()->unary_operation_->type_ = unary_operation::type::t_not;
						block[i]->assignment_.allowed_constant_type_ = constant_type::t_invalid;
						break;
					case bytecode::opcode::op_unm:
						block[i]->assignment_.expressions_.back()->unary_operation_->type_ = unary_operation::type::t_minus;
						block[i]->assignment_.allowed_constant_type_ = constant_type::t_bool;
						break;
					case bytecode::opcode::op_len:
						block[i]->assignment_.expressions_.back()->unary_operation_->type_ = unary_operation::type::t_length;
						break;
					default:
						break;
					}
					block[i]->assignment_.expressions_.back()->unary_operation_->operand_ = this->new_slot(block[i]->instruction_.d_);
					block[i]->assignment_.register_slots(block[i]->assignment_.expressions_.back()->unary_operation_->operand_);
					break;
				case bytecode::opcode::op_addvn:
				case bytecode::opcode::op_subvn:
				case bytecode::opcode::op_mulvn:
				case bytecode::opcode::op_divvn:
				case bytecode::opcode::op_modvn:
				case bytecode::opcode::op_addnv:
				case bytecode::opcode::op_subnv:
				case bytecode::opcode::op_mulnv:
				case bytecode::opcode::op_divnv:
				case bytecode::opcode::op_modnv:
				case bytecode::opcode::op_addvv:
				case bytecode::opcode::op_subvv:
				case bytecode::opcode::op_mulvv:
				case bytecode::opcode::op_divvv:
				case bytecode::opcode::op_modvv:
				case bytecode::opcode::op_pow:
					block[i]->assignment_.allowed_constant_type_ = constant_type::t_bool;
				case bytecode::opcode::op_cat:
					block[i]->assignment_.expressions_.back() = this->new_expression(ENUM_UNDER(expression::type::t_binary_operation));
					switch (block[i]->instruction_.type_) {
					case bytecode::opcode::op_addvn:
					case bytecode::opcode::op_addnv:
					case bytecode::opcode::op_addvv:
						block[i]->assignment_.expressions_.back()->binary_operation_->type_ = binary_operation::type::t_addition;
						break;
					case bytecode::opcode::op_subvn:
					case bytecode::opcode::op_subnv:
					case bytecode::opcode::op_subvv:
						block[i]->assignment_.expressions_.back()->binary_operation_->type_ = binary_operation::type::t_subtraction;
						break;
					case bytecode::opcode::op_mulvn:
					case bytecode::opcode::op_mulnv:
					case bytecode::opcode::op_mulvv:
						block[i]->assignment_.expressions_.back()->binary_operation_->type_ = binary_operation::type::t_multiplication;
						break;
					case bytecode::opcode::op_divvn:
					case bytecode::opcode::op_divnv:
					case bytecode::opcode::op_divvv:
						block[i]->assignment_.expressions_.back()->binary_operation_->type_ = binary_operation::type::t_division;
						break;
					case bytecode::opcode::op_modvn:
					case bytecode::opcode::op_modnv:
					case bytecode::opcode::op_modvv:
						block[i]->assignment_.expressions_.back()->binary_operation_->type_ = binary_operation::type::t_modulo;
						break;
					case bytecode::opcode::op_pow:
						block[i]->assignment_.expressions_.back()->binary_operation_->type_ = binary_operation::type::t_exponentiation;
						break;
					case bytecode::opcode::op_cat:
						block[i]->assignment_.expressions_.back()->binary_operation_->type_ = binary_operation::type::t_concatenation;
						break;
					default:
						break;
					}
					switch (block[i]->instruction_.type_) {
					case bytecode::opcode::op_addvn:
					case bytecode::opcode::op_subvn:
					case bytecode::opcode::op_mulvn:
					case bytecode::opcode::op_divvn:
					case bytecode::opcode::op_modvn:
						block[i]->assignment_.expressions_.back()->binary_operation_->left_operand_ = this->new_slot(block[i]->instruction_.b_);
						block[i]->assignment_.register_slots(block[i]->assignment_.expressions_.back()->binary_operation_->left_operand_);
						block[i]->assignment_.expressions_.back()->binary_operation_->right_operand_ = this->new_number(function_in,
							block[i]->instruction_.c_);
						break;
					case bytecode::opcode::op_addnv:
					case bytecode::opcode::op_subnv:
					case bytecode::opcode::op_mulnv:
					case bytecode::opcode::op_divnv:
					case bytecode::opcode::op_modnv:
						block[i]->assignment_.expressions_.back()->binary_operation_->left_operand_ = this->new_number(function_in,
							block[i]->instruction_.c_);
						block[i]->assignment_.expressions_.back()->binary_operation_->right_operand_ = this->new_slot(block[i]->instruction_.b_);
						block[i]->assignment_.register_slots(block[i]->assignment_.expressions_.back()->binary_operation_->right_operand_);
						break;
					case bytecode::opcode::op_addvv:
					case bytecode::opcode::op_subvv:
					case bytecode::opcode::op_mulvv:
					case bytecode::opcode::op_divvv:
					case bytecode::opcode::op_modvv:
					case bytecode::opcode::op_pow:
						block[i]->assignment_.expressions_.back()->binary_operation_->left_operand_ = this->new_slot(block[i]->instruction_.b_);
						block[i]->assignment_.expressions_.back()->binary_operation_->right_operand_ = this->new_slot(block[i]->instruction_.c_);
						block[i]->assignment_.register_slots(block[i]->assignment_.expressions_.back()->binary_operation_->left_operand_,
							block[i]->assignment_.expressions_.back()->binary_operation_->right_operand_);
						break;
					case bytecode::opcode::op_cat:
						block[i]->assignment_.expressions_.back()->binary_operation_->left_operand_ = this->new_slot(block[i]->instruction_.b_);
						for (expression* expr = block[i]->assignment_.expressions_.back(); true; expr = expr->binary_operation_->right_operand_) {
							block[i]->assignment_.register_slots(expr->binary_operation_->left_operand_);
							if (expr->binary_operation_->left_operand_->variable_->slot_ == block[i]->instruction_.c_ - 1) {
								expr->binary_operation_->right_operand_ = this->new_slot(block[i]->instruction_.c_);
								block[i]->assignment_.register_slots(expr->binary_operation_->right_operand_);
								break;
							}
							expr->binary_operation_->right_operand_ = this->new_expression(ENUM_UNDER(expression::type::t_binary_operation));
							expr->binary_operation_->right_operand_->binary_operation_->type_ = binary_operation::type::t_concatenation;
							expr->binary_operation_->right_operand_->binary_operation_->left_operand_
								= this->new_slot(expr->binary_operation_->left_operand_->variable_->slot_ + 1);
						}
						break;
					default:
						break;
					}
					break;
				case bytecode::opcode::op_kstr:
					block[i]->assignment_.expressions_.back() = this->new_string(function_in, block[i]->instruction_.d_);
					this->check_valid_name(block[i]->assignment_.expressions_.back()->constant_);
					break;
				case bytecode::opcode::op_khash:
					block[i]->assignment_.expressions_.back() = this->new_xhash(function_in, block[i]->instruction_.d_);
					break;
				case bytecode::opcode::op_kcdata:
					block[i]->assignment_.expressions_.back() = this->new_cdata(function_in, block[i]->instruction_.d_);
					break;
				case bytecode::opcode::op_kshort:
					block[i]->assignment_.expressions_.back() = this->new_signed_literal(block[i]->instruction_.d_);
					break;
				case bytecode::opcode::op_knum:
					block[i]->assignment_.expressions_.back() = this->new_number(function_in, block[i]->instruction_.d_);
					break;
				case bytecode::opcode::op_kpri:
					block[i]->assignment_.expressions_.back() = this->new_primitive(block[i]->instruction_.d_);
					break;
				case bytecode::opcode::op_knil:
					block[i]->assignment_.expressions_.back() = this->new_primitive(0);
					if (block[i]->instruction_.a_ == block[i]->instruction_.d_) {
						break;
					}
					block.emplace(block.begin() + i, this->new_statement(ENUM_UNDER(statement::type::t_instruction)));
					block[i]->instruction_ = block[i + 1]->instruction_;
					block[i]->instruction_.d_--;
					i++;
					block[i]->instruction_.a_ = block[i]->instruction_.d_;
					block[i]->instruction_.id_ = INVALID_ID;
					block[i]->instruction_.label_ = INVALID_ID;
					break;
				case bytecode::opcode::op_uget:
					block[i]->assignment_.expressions_.back() = this->new_expression(ENUM_UNDER(expression::type::t_variable));
					block[i]->assignment_.expressions_.back()->variable_->type_ = variable::type::t_upvalue;
					block[i]->assignment_.expressions_.back()->variable_->slot_scope_ = function_in.upvalues_[block[i]->instruction_.d_].slot_scope_;
					break;
				case bytecode::opcode::op_usetv:
				case bytecode::opcode::op_usets:
				case bytecode::opcode::op_usetn:
				case bytecode::opcode::op_usetp:
					block[i]->assignment_.variables_.resize(1);
					block[i]->assignment_.variables_.back().type_ = variable::type::t_upvalue;
					block[i]->assignment_.variables_.back().slot_scope_ = function_in.upvalues_[block[i]->instruction_.a_].slot_scope_;
					switch (block[i]->instruction_.type_) {
					case bytecode::opcode::op_usetv:
						block[i]->assignment_.expressions_.back() = this->new_slot(block[i]->instruction_.d_);
						block[i]->assignment_.register_slots(block[i]->assignment_.expressions_.back());
						break;
					case bytecode::opcode::op_usets:
						block[i]->assignment_.expressions_.back() = this->new_string(function_in, block[i]->instruction_.d_);
						break;
					case bytecode::opcode::op_usetn:
						block[i]->assignment_.expressions_.back() = this->new_number(function_in, block[i]->instruction_.d_);
						break;
					case bytecode::opcode::op_usetp:
						block[i]->assignment_.expressions_.back() = this->new_primitive(block[i]->instruction_.d_);
						break;
					default:
						break;
					}
					continue;
				case bytecode::opcode::op_fnew:
					block[i]->assignment_.expressions_.back() = this->new_expression(ENUM_UNDER(expression::type::t_function));
					block[i]->assignment_.expressions_.back()->function_ = block[i]->function_;
					break;
				case bytecode::opcode::op_tnew:
					block[i]->assignment_.expressions_.back() = this->new_expression(ENUM_UNDER(expression::type::t_table));
					block[i]->assignment_.is_table_constructor_ = true;
					break;
				case bytecode::opcode::op_tdup:
					block[i]->assignment_.expressions_.back() = this->new_table(function_in, block[i]->instruction_.d_);
					block[i]->assignment_.is_table_constructor_ = true;
					break;
				case bytecode::opcode::op_gget:
					block[i]->assignment_.expressions_.back() = this->new_expression(ENUM_UNDER(expression::type::t_variable));
					block[i]->assignment_.expressions_.back()->variable_->type_ = variable::type::t_global;
					block[i]->assignment_.expressions_.back()->variable_->name_ = function_in.get_constant(block[i]->instruction_.d_).string_;
					if (function_in.has_debug_info_) {
						function_in.used_globals_.emplace_back(&function_in.get_constant(block[i]->instruction_.d_).string_);
					}
					break;
				case bytecode::opcode::op_gset:
					block[i]->assignment_.variables_.resize(1);
					block[i]->assignment_.variables_.back().type_ = variable::type::t_global;
					block[i]->assignment_.variables_.back().name_ = function_in.get_constant(block[i]->instruction_.d_).string_;
					if (function_in.has_debug_info_) {
						function_in.used_globals_.emplace_back(&function_in.get_constant(block[i]->instruction_.d_).string_);
					}
					block[i]->assignment_.expressions_.back() = new_slot(block[i]->instruction_.a_);
					block[i]->assignment_.register_slots(block[i]->assignment_.expressions_.back());
					continue;
				case bytecode::opcode::op_tgetv:
				case bytecode::opcode::op_tgets:
				case bytecode::opcode::op_tgetb:
					block[i]->assignment_.expressions_.back() = this->new_expression(ENUM_UNDER(expression::type::t_variable));
					block[i]->assignment_.expressions_.back()->variable_->type_ = variable::type::t_table_index;
					block[i]->assignment_.expressions_.back()->variable_->table_ = this->new_slot(block[i]->instruction_.b_);
					block[i]->assignment_.register_slots(block[i]->assignment_.expressions_.back()->variable_->table_);
					switch (block[i]->instruction_.type_) {
					case bytecode::opcode::op_tgetv:
						block[i]->assignment_.expressions_.back()->variable_->table_index_ = this->new_slot(block[i]->instruction_.c_);
						block[i]->assignment_.register_slots(block[i]->assignment_.expressions_.back()->variable_->table_index_);
						break;
					case bytecode::opcode::op_tgets:
						block[i]->assignment_.expressions_.back()->variable_->table_index_ = this->new_string(function_in, block[i]->instruction_.c_);
						this->check_valid_name(block[i]->assignment_.expressions_.back()->variable_->table_index_->constant_);
						break;
					case bytecode::opcode::op_tgetb:
						block[i]->assignment_.expressions_.back()->variable_->table_index_ = this->new_literal(block[i]->instruction_.c_);
						break;
					default:
						break;
					}
					break;
				case bytecode::opcode::op_tsetv:
				case bytecode::opcode::op_tsets:
				case bytecode::opcode::op_tsetb:
					block[i]->assignment_.variables_.resize(1);
					block[i]->assignment_.variables_.back().type_ = variable::type::t_table_index;
					block[i]->assignment_.variables_.back().table_ = this->new_slot(block[i]->instruction_.b_);
					switch (block[i]->instruction_.type_) {
					case bytecode::opcode::op_tsetv:
						block[i]->assignment_.variables_.back().table_index_ = this->new_slot(block[i]->instruction_.c_);
						block[i]->assignment_.register_slots(block[i]->assignment_.variables_.back().table_index_);
						break;
					case bytecode::opcode::op_tsets:
						block[i]->assignment_.variables_.back().table_index_ = this->new_string(function_in, block[i]->instruction_.c_);
						this->check_valid_name(block[i]->assignment_.variables_.back().table_index_->constant_);
						break;
					case bytecode::opcode::op_tsetb:
						block[i]->assignment_.variables_.back().table_index_ = this->new_literal(block[i]->instruction_.c_);
						break;
					default:
						break;
					}
					block[i]->assignment_.expressions_.back() = this->new_slot(block[i]->instruction_.a_);
					block[i]->assignment_.register_slots(block[i]->assignment_.expressions_.back());
					continue;
				case bytecode::opcode::op_tsetm:
					block[i]->assignment_.variables_.resize(1);
					block[i]->assignment_.variables_.back().type_ = variable::type::t_table_index;
					block[i]->assignment_.variables_.back().is_multres_ = true;
					block[i]->assignment_.variables_.back().table_ = this->new_slot(block[i]->instruction_.a_ - 1);
					ld_assert(function_in.get_number_constant(block[i]->instruction_.d_).type_ == bytecode::number_constant::type::t_num
						&& VAL_AS(std::uint32_t, function_in.get_number_constant(block[i]->instruction_.d_).number_),
						"Multres table index is not a valid number constant", this->bc_.file_path_);
					block[i]->assignment_.variables_.back().multres_index_ = function_in.get_number_constant(block[i]->instruction_.d_).number_;
					block[i]->assignment_.expressions_.back() = this->new_slot(block[i]->instruction_.a_);
					block[i]->assignment_.expressions_.back()->variable_->is_multres_ = true;
					block[i]->assignment_.register_slots(block[i]->assignment_.expressions_.back());
					continue;
				case bytecode::opcode::op_callm:
				case bytecode::opcode::op_call:
					block[i]->assignment_.expressions_.back() = this->new_expression(ENUM_UNDER(expression::type::t_function_call));
					if (block[i]->instruction_.b_) {
						if (block[i]->instruction_.b_ == 1) {
							block[i]->type_ = statement::type::t_function_call;
						}
						else {
							block[i]->assignment_.variables_.resize(block[i]->instruction_.b_ - 1);
							for (std::uint8_t j = block[i]->assignment_.variables_.size(); j--;) {
								block[i]->assignment_.variables_[j].type_ = variable::type::t_slot;
								block[i]->assignment_.variables_[j].slot_ = block[i]->instruction_.a_ + j;
							}
							block[i]->assignment_.expressions_.back()->function_call_->return_count_ = block[i]->assignment_.variables_.size();
						}
					}
					else {
						block[i]->assignment_.variables_.resize(1);
						block[i]->assignment_.variables_.back().type_ = variable::type::t_slot;
						block[i]->assignment_.variables_.back().slot_ = block[i]->instruction_.a_;
						block[i]->assignment_.variables_.back().is_multres_ = true;
					}
					block[i]->assignment_.expressions_.back()->function_call_->function_ = this->new_slot(block[i]->instruction_.a_);
					block[i]->assignment_.register_slots(block[i]->assignment_.expressions_.back()->function_call_->function_);
					block[i]->assignment_.expressions_.back()->function_call_->arguments_.resize(block[i]->instruction_.c_
						+ (block[i]->instruction_.type_ == bytecode::opcode::op_callm ? 0 : -1), nullptr);
					if (block[i]->assignment_.expressions_.back()->function_call_->arguments_.size()) {
						block[i]->assignment_.is_potential_method_ = true;
					}
					for (std::uint8_t j = 0; j < block[i]->assignment_.expressions_.back()->function_call_->arguments_.size(); j++) {
						block[i]->assignment_.expressions_.back()->function_call_->arguments_[j] = this->new_slot(block[i]->instruction_.a_
							+ (this->is_fr2_enabled_ ? 2 : 1) + j);
						block[i]->assignment_.register_slots(block[i]->assignment_.expressions_.back()->function_call_->arguments_[j]);
					}
					if (block[i]->instruction_.type_ == bytecode::opcode::op_callm) {
						block[i]->assignment_.expressions_.back()->function_call_->multres_argument_ = this->new_slot(block[i]->instruction_.a_
							+ (this->is_fr2_enabled_ ? 2 : 1) + block[i]->instruction_.c_);
						block[i]->assignment_.expressions_.back()->function_call_->multres_argument_->variable_->is_multres_ = true;
						block[i]->assignment_.register_slots(block[i]->assignment_.expressions_.back()->function_call_->multres_argument_);
					}
					continue;
				case bytecode::opcode::op_varg:
					block[i]->assignment_.expressions_.back() = this->new_expression(ENUM_UNDER(expression::type::t_vararg));
					if (block[i]->instruction_.b_) {
						if (block[i]->instruction_.b_ == 1) {
							block[i]->type_ = statement::type::t_function_call;
						}
						else {
							block[i]->assignment_.variables_.resize(block[i]->instruction_.b_ - 1);
							for (std::uint8_t j = block[i]->assignment_.variables_.size(); j--;) {
								block[i]->assignment_.variables_[j].type_ = variable::type::t_slot;
								block[i]->assignment_.variables_[j].slot_ = block[i]->instruction_.a_ + j;
							}
							block[i]->assignment_.expressions_.back()->return_count_ = block[i]->assignment_.variables_.size();
						}
					}
					else {
						block[i]->assignment_.variables_.resize(1);
						block[i]->assignment_.variables_.back().type_ = variable::type::t_slot;
						block[i]->assignment_.variables_.back().slot_ = block[i]->instruction_.a_;
						block[i]->assignment_.variables_.back().is_multres_ = true;
					}
					continue;
				default:
					break;
				}
				block[i]->assignment_.variables_.resize(1);
				block[i]->assignment_.variables_.back().type_ = variable::type::t_slot;
				block[i]->assignment_.variables_.back().slot_ = block[i]->instruction_.a_;
				continue;
			case statement::type::t_return:
				if (i
					&& block[i - 1]->type_ == statement::type::t_empty
					&& block[i - 1]->instruction_.type_ == bytecode::opcode::op_uclo
					&& !function_in.is_valid_label(block[i]->instruction_.label_))
				{
					block[i]->instruction_.id_ = block[i - 1]->instruction_.id_;
					block[i]->instruction_.label_ = block[i - 1]->instruction_.label_;
					i--;
					block.erase(block.begin() + i);
				}
				switch (block[i]->instruction_.type_) {
				case bytecode::opcode::op_callmt:
				case bytecode::opcode::op_callt:
					block[i]->assignment_.multres_return_ = this->new_expression(ENUM_UNDER(expression::type::t_function_call));
					block[i]->assignment_.multres_return_->function_call_->function_ = this->new_slot(block[i]->instruction_.a_);
					block[i]->assignment_.register_slots(block[i]->assignment_.multres_return_->function_call_->function_);
					block[i]->assignment_.multres_return_->function_call_->arguments_.resize(block[i]->instruction_.d_
						+ (block[i]->instruction_.type_ == bytecode::opcode::op_callmt ? 0 : -1), nullptr);
					if (block[i]->assignment_.multres_return_->function_call_->arguments_.size()) {
						block[i]->assignment_.is_potential_method_ = true;
					}
					for (std::uint8_t j = 0; j < block[i]->assignment_.multres_return_->function_call_->arguments_.size(); j++) {
						block[i]->assignment_.multres_return_->function_call_->arguments_[j] = this->new_slot(block[i]->instruction_.a_
							+ (this->is_fr2_enabled_ ? 2 : 1) + j);
						block[i]->assignment_.register_slots(block[i]->assignment_.multres_return_->function_call_->arguments_[j]);
					}
					if (block[i]->instruction_.type_ == bytecode::opcode::op_callmt) {
						block[i]->assignment_.multres_return_->function_call_->multres_argument_ = this->new_slot(block[i]->instruction_.a_
							+ (this->is_fr2_enabled_ ? 2 : 1) + block[i]->instruction_.d_);
						block[i]->assignment_.multres_return_->function_call_->multres_argument_->variable_->is_multres_ = true;
						block[i]->assignment_.register_slots(block[i]->assignment_.multres_return_->function_call_->multres_argument_);
					}
					break;
				case bytecode::opcode::op_retm:
				case bytecode::opcode::op_ret:
				case bytecode::opcode::op_ret1:
					block[i]->assignment_.expressions_.resize(block[i]->instruction_.type_ == bytecode::opcode::op_ret1
						? 1 : block[i]->instruction_.d_ + (block[i]->instruction_.type_ == bytecode::opcode::op_retm ? 0 : -1), nullptr);
					for (auto j = 0; j < block[i]->assignment_.expressions_.size(); j++) {
						block[i]->assignment_.expressions_[j] = this->new_slot(block[i]->instruction_.a_ + j);
						block[i]->assignment_.register_slots(block[i]->assignment_.expressions_[j]);
					}
					if (block[i]->instruction_.type_ == bytecode::opcode::op_retm) {
						block[i]->assignment_.multres_return_ = this->new_slot(block[i]->instruction_.a_ + block[i]->instruction_.d_);
						block[i]->assignment_.multres_return_->variable_->is_multres_ = true;
						block[i]->assignment_.register_slots(block[i]->assignment_.multres_return_);
					}
					break;
				default:
					break;
				}
				continue;
			case statement::type::t_condition:
				switch (block[i]->instruction_.type_) {
				case bytecode::opcode::op_islt:
				case bytecode::opcode::op_isge:
				case bytecode::opcode::op_isle:
				case bytecode::opcode::op_isgt:
				case bytecode::opcode::op_iseqv:
				case bytecode::opcode::op_isnev:
				case bytecode::opcode::op_iseqs:
				case bytecode::opcode::op_isnes:
				case bytecode::opcode::op_iseqn:
				case bytecode::opcode::op_isnen:
				case bytecode::opcode::op_iseqp:
				case bytecode::opcode::op_isnep:
					block[i]->assignment_.expressions_.resize(2, nullptr);
					block[i]->assignment_.expressions_[0] = this->new_slot(block[i]->instruction_.a_);
					block[i]->assignment_.register_slots(block[i]->assignment_.expressions_[0]);
					switch (block[i]->instruction_.type_) {
					case bytecode::opcode::op_islt:
					case bytecode::opcode::op_isge:
					case bytecode::opcode::op_isle:
					case bytecode::opcode::op_isgt:
						block[i]->condition_.allow_slot_swap_ = true;
					case bytecode::opcode::op_iseqv:
					case bytecode::opcode::op_isnev:
						block[i]->assignment_.expressions_[1] = this->new_slot(block[i]->instruction_.d_);
						block[i]->assignment_.register_slots(block[i]->assignment_.expressions_[1]);
						break;
					case bytecode::opcode::op_iseqs:
					case bytecode::opcode::op_isnes:
						block[i]->assignment_.expressions_[1] = this->new_string(function_in, block[i]->instruction_.d_);
						break;
					case bytecode::opcode::op_iseqn:
					case bytecode::opcode::op_isnen:
						block[i]->assignment_.expressions_[1] = this->new_number(function_in, block[i]->instruction_.d_);
						break;
					case bytecode::opcode::op_iseqp:
					case bytecode::opcode::op_isnep:
						block[i]->assignment_.expressions_[1] = this->new_primitive(block[i]->instruction_.d_);
						break;
					default:
						break;
					}
					break;
				case bytecode::opcode::op_istc:
				case bytecode::opcode::op_isfc:
					block[i]->assignment_.variables_.resize(1);
					block[i]->assignment_.variables_.back().type_ = variable::type::t_slot;
					block[i]->assignment_.variables_.back().slot_ = block[i]->instruction_.a_;
				case bytecode::opcode::op_ist:
				case bytecode::opcode::op_isf:
					block[i]->assignment_.expressions_.resize(1, this->new_slot(block[i]->instruction_.d_));
					block[i]->assignment_.register_slots(block[i]->assignment_.expressions_.back());
					block[i]->assignment_.allowed_constant_type_ = constant_type::t_invalid;
					break;
				default:
					break;
				}
				continue;
			case statement::type::t_numeric_for:
				block[i]->assignment_.variables_.resize(1);
				block[i]->assignment_.variables_.back().type_ = variable::type::t_slot;
				block[i]->assignment_.variables_.back().slot_ = block[i]->instruction_.a_ + 3;
				ld_assert(!function_in.has_debug_info_
					|| (block[i]->locals_
						&& block[i]->assignment_.variables_.back().slot_ == block[i]->locals_->base_slot_
						&& block[i]->locals_->names_.size() == 1),
					"Numeric for loop variable does not match with debug info", this->bc_.file_path_);
				block[i]->assignment_.expressions_.resize(3, nullptr);
				block[i]->assignment_.expressions_[0] = this->new_slot(block[i]->instruction_.a_);
				block[i]->assignment_.expressions_[1] = this->new_slot(block[i]->instruction_.a_ + 1);
				block[i]->assignment_.expressions_[2] = this->new_slot(block[i]->instruction_.a_ + 2);
				block[i]->assignment_.register_slots(block[i]->assignment_.expressions_[0], block[i]->assignment_.expressions_[1],
					block[i]->assignment_.expressions_[2]);
				continue;
			case statement::type::t_generic_for:
				block[i]->assignment_.variables_.resize(block[i]->instruction_.b_ - 1);
				for (auto j = block[i]->assignment_.variables_.size(); j--;) {
					block[i]->assignment_.variables_[j].type_ = variable::type::t_slot;
					block[i]->assignment_.variables_[j].slot_ = block[i]->instruction_.a_ + j;
				}
				ld_assert(!function_in.has_debug_info_
					|| (block[i]->locals_
						&& block[i]->assignment_.variables_.front().slot_ == block[i]->locals_->base_slot_
						&& block[i]->locals_->names_.size() == block[i]->assignment_.variables_.size()),
					"Generic for loop variables do not match with debug info", this->bc_.file_path_);
				block[i]->assignment_.expressions_.resize(3, nullptr);
				block[i]->assignment_.expressions_[0] = this->new_slot(block[i]->instruction_.a_ - 3);
				block[i]->assignment_.expressions_[1] = this->new_slot(block[i]->instruction_.a_ - 2);
				block[i]->assignment_.expressions_[2] = this->new_slot(block[i]->instruction_.a_ - 1);
				block[i]->assignment_.register_slots(block[i]->assignment_.expressions_[0], block[i]->assignment_.expressions_[1],
					block[i]->assignment_.expressions_[2]);
				continue;
			case statement::type::t_declaration:
				block[i]->assignment_.variables_.resize(block[i]->locals_->names_.size());
				block[i]->assignment_.expressions_.resize(block[i]->assignment_.variables_.size(), nullptr);
				for (auto j = 0; j < block[i]->assignment_.variables_.size(); j++) {
					block[i]->assignment_.variables_[j].type_ = variable::type::t_slot;
					block[i]->assignment_.variables_[j].slot_ = block[i]->locals_->base_slot_ + j;
					block[i]->assignment_.expressions_[j] = this->new_slot(block[i]->assignment_.variables_[j].slot_);
					block[i]->assignment_.register_slots(block[i]->assignment_.expressions_[j]);
				}
				continue;
			default:
				break;
			}
		}
	}

	void ast::build_slot_scopes(function& function_in, std::vector<statement*>& block, block_info* const& previous_block) {
		const auto build_nil_assignment = [this](const std::uint8_t& slot) -> statement* const {
			statement* const st = this->new_statement(ENUM_UNDER(statement::type::t_assignment));
			st->assignment_.expressions_.resize(1, this->new_primitive(0));
			st->assignment_.variables_.resize(1);
			st->assignment_.variables_.back().type_ = variable::type::t_slot;
			st->assignment_.variables_.back().slot_ = slot;
			return st;
		};

		block_info info = {
			.block_ = block,
			.previous_block_ = previous_block
		};
		std::uint32_t id, index, target_label, extended_target_label;
		std::uint8_t target_slot;
		slot_scope** target_slot_scope;
		bool is_possible_condition;
		bool has_bool_construct;
		std::vector<std::vector<statement*>> condition_blocks;

		for (std::uint32_t i = block.size(); i--;) {
			switch (block[i]->type_) {
			case statement::type::t_numeric_for:
			case statement::type::t_generic_for:
				for (auto j = block[i]->assignment_.variables_.size(); j--;) {
					ld_assert(!function_in.slot_scope_collector_.slot_infos_[block[i]->assignment_.variables_[j].slot_].active_slot_scope_,
						"Slot scope does not match with for loop variable", this->bc_.file_path_);
					function_in.slot_scope_collector_.begin_scope(block[i]->assignment_.variables_[j].slot_, block[i]->instruction_.target_ - 1);
				}
			case statement::type::t_loop:
				function_in.slot_scope_collector_.extend_scopes(block[i]->instruction_.id_);
				info.index_ = i;
				this->build_slot_scopes(function_in, block[i]->block_, block[i]->type_ == statement::type::t_loop ? &info : nullptr);
				function_in.slot_scope_collector_.merge_scopes(block[i]->instruction_.target_ - 1);
				break;
			case statement::type::t_declaration:
				block[i]->instruction_.id_ = INVALID_ID;

				for (auto j = function_in.slot_scope_collector_.slot_infos_.size(); j-- && j >= block[i]->locals_->base_slot_;) {
					if (!function_in.slot_scope_collector_.slot_infos_[j].active_slot_scope_) {
						continue;
					}

					for (auto k = j; true; k--) {
						ld_assert(function_in.slot_scope_collector_.slot_infos_[k].active_slot_scope_
							&& function_in.slot_scope_collector_.slot_infos_[k].min_scope_begin_ == INVALID_ID,
							"Slot scope does not match with variable debug info", this->bc_.file_path_);
						block.emplace(block.begin() + i + 1, build_nil_assignment(k));
						function_in.slot_scope_collector_.close_scope(k, block[i + 1]->assignment_.variables_.back().slot_scope_, block[i]->locals_->scope_end_);
						if (k == block[i]->locals_->base_slot_) {
							break;
						}
					}

					break;
				}

				for (auto j = block[i]->assignment_.variables_.size(); j--;) {
					function_in.slot_scope_collector_.begin_scope(block[i]->assignment_.variables_[j].slot_, block[i]->locals_->scope_end_);
				}

				function_in.slot_scope_collector_.extend_scopes(block[i]->locals_->scope_begin_);
				info.index_ = i;
				this->build_slot_scopes(function_in, block[i]->block_, &info);

				for (auto j = function_in.slot_scope_collector_.slot_infos_.size(); j-- && j >= block[i]->assignment_.variables_.back().slot_ + 1;) {
					if (!function_in.slot_scope_collector_.slot_infos_[j].active_slot_scope_) {
						continue;
					}

					for (auto k = j; true; k--) {
						ld_assert(function_in.slot_scope_collector_.slot_infos_[k].active_slot_scope_
							&& function_in.slot_scope_collector_.slot_infos_[k].min_scope_begin_ == INVALID_ID,
							"Slot scope does not match with variable debug info", this->bc_.file_path_);
						block[i]->block_.emplace(block[i]->block_.begin(), build_nil_assignment(k));
						function_in.slot_scope_collector_.close_scope(k, block[i]->block_.front()->assignment_.variables_.back().slot_scope_, block[i]->locals_->scope_begin_);
						if (k == block[i]->assignment_.variables_.back().slot_ + 1) {
							break;
						}
					}

					break;
				}

				break;
			default:
				break;
			}

			if (block[i]->instruction_.id_ != INVALID_ID) {
				id = block[i]->instruction_.id_;
				info.index_ = i;
				target_label = this->get_label_from_next_statement(function_in, info, false, true);
				extended_target_label = this->get_label_from_next_statement(function_in, info, true, true);

				if (function_in.is_valid_label(target_label) && function_in.labels_[target_label].jump_ids_.front() < id
					&& (extended_target_label == target_label
						|| function_in.labels_[extended_target_label].target_ > id
						|| function_in.labels_[extended_target_label].target_ < function_in.labels_[target_label].jump_ids_.front()))
				{
					index = this->get_block_index_from_id(block, function_in.labels_[target_label].jump_ids_.front() - 1);

					if (index != INVALID_ID) {
						is_possible_condition = false;
						has_bool_construct = false;

						switch (block[i]->type_) {
						case statement::type::t_condition:
							if (!block[i]->assignment_.variables_.size() && block[i]->instruction_.target_ == function_in.labels_[extended_target_label].target_) {
								switch (block[index]->type_) {
								case statement::type::t_condition:
									if (block[index]->assignment_.expressions_.size() == 1) {
										if (block[index]->assignment_.variables_.size()) {
											if (function_in.slot_scope_collector_.slot_infos_[block[index]->assignment_.variables_.back().slot_].active_slot_scope_
												&& function_in.slot_scope_collector_.slot_infos_[block[index]->assignment_.variables_.back().slot_].min_scope_begin_ == block[index]->instruction_.id_)
											{
												is_possible_condition = true;
												target_slot = block[index]->assignment_.variables_.back().slot_;
											}
										}
										else if (function_in.slot_scope_collector_.slot_infos_[block[index]->assignment_.expressions_.back()->variable_->slot_].active_slot_scope_
											&& function_in.slot_scope_collector_.slot_infos_[block[index]->assignment_.expressions_.back()->variable_->slot_].min_scope_begin_ == block[index]->instruction_.id_)
										{
											is_possible_condition = true;
											target_slot = block[index]->assignment_.expressions_.back()->variable_->slot_;
										}
									}

									break;
								case statement::type::t_assignment:
									if (block[index]->assignment_.variables_.size() == 1
										&& block[index]->assignment_.variables_.back().type_ == variable::type::t_slot
										&& function_in.slot_scope_collector_.slot_infos_[block[index]->assignment_.variables_.back().slot_].active_slot_scope_
										&& function_in.slot_scope_collector_.slot_infos_[block[index]->assignment_.variables_.back().slot_].min_scope_begin_ == block[index]->instruction_.id_
										&& get_constant_type(block[index]->assignment_.expressions_.back()) != constant_type::t_invalid)
									{
										is_possible_condition = true;
										target_slot = block[index]->assignment_.variables_.back().slot_;
									}

									break;
								default:
									break;
								}
							}

							break;
						case statement::type::t_assignment:
							if (block[i]->assignment_.variables_.size() == 1) {
								switch (block[i]->assignment_.variables_.back().type_) {
								case variable::type::t_slot:
									if (function_in.slot_scope_collector_.slot_infos_[block[i]->assignment_.variables_.back().slot_].active_slot_scope_
										&& function_in.slot_scope_collector_.slot_infos_[block[i]->assignment_.variables_.back().slot_].min_scope_begin_ == block[index]->instruction_.id_)
									{
										is_possible_condition = true;
										target_slot = block[i]->assignment_.variables_.back().slot_;
										if (i >= 5
											&& index <= i - 4
											&& (((block[i - 3]->type_ == statement::type::t_goto
														|| block[i - 3]->type_ == statement::type::t_break)
													&& !function_in.is_valid_label(block[i - 3]->instruction_.label_)
													&& block[i - 3]->instruction_.target_ == function_in.labels_[extended_target_label].target_)
												|| (block[i - 3]->type_ == statement::type::t_condition
													&& block[i - 3]->assignment_.expressions_.size() == 2
													&& block[i - 3]->instruction_.target_ == block[i]->instruction_.id_))
											&& block[i]->assignment_.expressions_.back()->type_ == expression::type::t_constant
											&& block[i]->assignment_.expressions_.back()->constant_->type_ == constant::type::t_true
											&& (block[i - 1]->type_ == statement::type::t_goto
												|| block[i - 1]->type_ == statement::type::t_break)
											&& !function_in.is_valid_label(block[i - 1]->instruction_.label_)
											&& block[i - 1]->instruction_.target_ == function_in.labels_[target_label].target_
											&& block[i - 2]->type_ == statement::type::t_assignment
											&& block[i - 2]->assignment_.expressions_.back()->type_ == expression::type::t_constant
											&& block[i - 2]->assignment_.expressions_.back()->constant_->type_ == constant::type::t_false
											&& (function_in.is_valid_label(block[i]->instruction_.label_)
												|| function_in.is_valid_label(block[i - 2]->instruction_.label_)))
										{
											has_bool_construct = true;
										}
									}

									break;
								case variable::type::t_table_index:
									if (function_in.slot_scope_collector_.slot_infos_[block[i]->assignment_.variables_.back().table_->variable_->slot_].active_slot_scope_
										&& function_in.slot_scope_collector_.slot_infos_[block[i]->assignment_.variables_.back().table_->variable_->slot_].min_scope_begin_ == block[index]->instruction_.id_)
									{
										is_possible_condition = true;
										target_slot = block[i]->assignment_.variables_.back().table_->variable_->slot_;
									}

									break;
								default:
									break;
								}
							}

							break;
						default:
							break;
						}

						if (is_possible_condition) {
							condition_blocks.clear();

							if (has_bool_construct) {
								condition_blocks.resize(2);
								condition_blocks[0].emplace_back(block[i]);
								condition_blocks[1].emplace_back(block[i - 2]);
								condition_blocks[1].emplace_back(block[i - 1]);
								index = block[i - 3]->type_ == statement::type::t_condition ? i - 3 : i - 4;
							} else {
								index = i;
							}

							if (!has_bool_construct || index == i - 4) {
								is_possible_condition = false;

								if (block[index]->type_ == statement::type::t_assignment
									&& block[index]->assignment_.variables_.size() == 1
									&& block[index]->assignment_.variables_.back().type_ == variable::type::t_slot)
								{
									if (block[index]->assignment_.variables_.back().slot_ == target_slot) {
										is_possible_condition = true;
									}
								}
								else if ((block[index]->type_ == statement::type::t_assignment
										&& block[index]->assignment_.variables_.size() == 1
										&& block[index]->assignment_.variables_.back().type_ == variable::type::t_table_index
										&& block[index]->assignment_.variables_.back().table_->variable_->slot_ == target_slot)
									|| (block[index]->type_ == statement::type::t_condition
										&& block[index]->instruction_.target_ == function_in.labels_[extended_target_label].target_
										&& !block[index]->assignment_.variables_.size())) {
									while (index--) {
										switch (block[index]->type_) {
										case statement::type::t_condition:
											if (!block[index]->assignment_.variables_.size() && block[index]->instruction_.target_ == function_in.labels_[extended_target_label].target_) {
												continue;
											}
										case statement::type::t_goto:
										case statement::type::t_break:
											if (block[index]->instruction_.target_ == function_in.labels_[target_label].target_
												|| block[index]->instruction_.target_ == function_in.labels_[extended_target_label].target_
												|| block[index]->instruction_.target_ > block[has_bool_construct ? i - 4 : i]->instruction_.id_)
												break;
											continue;
										case statement::type::t_assignment:
											if (block[index]->assignment_.variables_.size() == 1
												&& block[index]->assignment_.variables_.back().type_ == variable::type::t_slot
												&& block[index]->assignment_.variables_.back().slot_ == target_slot) {
												if (block[index]->assignment_.is_table_constructor_
													&& (has_bool_construct
														|| block[index]->instruction_.id_ > function_in.labels_[target_label].jump_ids_.front())
													&& function_in.is_valid_block_range(block[index]->instruction_.id_, block[has_bool_construct ? i - 4 : i]->instruction_.id_, true))
												{
													is_possible_condition = true;
												}
												break;
											}
										default:
											continue;
										}

										break;
									}
								}
							}

							for (auto blockIndex = has_bool_construct ? i - 3 : i; is_possible_condition;) {
								if (block[index]->instruction_.id_ < function_in.labels_[target_label].jump_ids_.front()) {
									condition_blocks.emplace_back(block.begin() + index, block.begin() + blockIndex + 1);
									break;
								}

								is_possible_condition = false;

								while (index--) {
									switch (block[index]->type_) {
										case statement::type::t_condition:
										case statement::type::t_goto:
										case statement::type::t_break:
											if (block[index]->instruction_.target_ == function_in.labels_[target_label].target_) {
												break;
											}
										default:
											continue;
									}

									condition_blocks.emplace_back(block.begin() + index + 1, block.begin() + blockIndex + 1);
									blockIndex = index;

									switch (block[index]->type_) {
									case statement::type::t_condition:
										if (block[index]->assignment_.expressions_.size() != 1) {
											break;
										}
										if (block[index]->assignment_.variables_.size()) {
											if (block[index]->assignment_.variables_.back().slot_ == target_slot) {
												is_possible_condition = true;
											}
										}
										else if (index && block[index]->assignment_.expressions_.back()->variable_->slot_ == target_slot) {
											index--;

											if (block[index]->type_ == statement::type::t_assignment
												&& block[index]->assignment_.variables_.size() == 1
												&& block[index]->assignment_.variables_.back().type_ == variable::type::t_slot)
											{
												if (block[index]->assignment_.variables_.back().slot_ == target_slot && !function_in.is_valid_label(block[index + 1]->instruction_.label_)) {
													is_possible_condition = true;
												}
											}
											else if ((block[index]->type_ == statement::type::t_assignment
													&& block[index]->assignment_.variables_.size() == 1
													&& block[index]->assignment_.variables_.back().type_ == variable::type::t_table_index
													&& block[index]->assignment_.variables_.back().table_->variable_->slot_ == target_slot
													&& !function_in.is_valid_label(block[index + 1]->instruction_.label_))
												|| (block[index]->type_ == statement::type::t_condition
													&& block[index]->instruction_.target_ == block[blockIndex]->instruction_.id_
													&& !block[index]->assignment_.variables_.size()))
											{
												while (index--) {
													switch (block[index]->type_) {
													case statement::type::t_condition:
														if (!block[index]->assignment_.variables_.size() && block[index]->instruction_.target_ == block[blockIndex]->instruction_.id_) {
															continue;
														}
													case statement::type::t_goto:
													case statement::type::t_break:
														if (block[index]->instruction_.target_ == function_in.labels_[target_label].target_
															|| block[index]->instruction_.target_ == function_in.labels_[extended_target_label].target_
															|| block[index]->instruction_.target_ >= block[blockIndex]->instruction_.id_)
														{
															break;
														}
														continue;
													case statement::type::t_assignment:
														if (block[index]->assignment_.variables_.size() == 1
															&& block[index]->assignment_.variables_.back().type_ == variable::type::t_slot
															&& block[index]->assignment_.variables_.back().slot_ == target_slot)
														{
															if (block[index]->assignment_.is_table_constructor_
																&& function_in.is_valid_block_range(block[index]->instruction_.id_, block[blockIndex]->instruction_.id_, true))
															{
																is_possible_condition = true;
															}
															break;
														}
													default:
														continue;
													}

													break;
												}
											}
										}

										break;
									case statement::type::t_goto:
									case statement::type::t_break:
										if (index--
											&& block[index]->assignment_.variables_.size() == 1
											&& block[index]->assignment_.variables_.back().type_ == variable::type::t_slot
											&& block[index]->assignment_.variables_.back().slot_ == target_slot
											&& get_constant_type(block[index]->assignment_.expressions_.back()) != constant_type::t_invalid)
										{
											is_possible_condition = true;
										}
										break;
									default:
										break;
									}

									break;
								}
							}

							if (is_possible_condition) {
								for (auto j = index; j <= i; j++) {
									switch (block[j]->type_) {
									case statement::type::t_assignment:
										if (block[j]->assignment_.variables_.size() == 1) {
											switch (block[j]->assignment_.variables_.back().type_) {
											case variable::type::t_slot:
												if (block[j]->function_) {
													for (std::uint8_t k = block[j]->function_->upvalues_.size(); k--;) {
														if (!block[j]->function_->upvalues_[k].local_ || block[j]->function_->upvalues_[k].slot_ != target_slot) {
															continue;
														}
														is_possible_condition = false;
														break;
													}
													if (!is_possible_condition) {
														break;
													}
												}
											case variable::type::t_table_index:
												continue;
											default:
												break;
											}
										}
									case statement::type::t_empty:
									case statement::type::t_return:
									case statement::type::t_numeric_for:
									case statement::type::t_generic_for:
									case statement::type::t_loop:
									case statement::type::t_declaration:
									case statement::type::t_function_call:
										break;
									case statement::type::t_goto:
									case statement::type::t_break:
										if (function_in.is_valid_label(block[j]->instruction_.label_) || block[j]->instruction_.type_ != bytecode::opcode::op_jmp) {
											break;
										}
									case statement::type::t_condition:
										if (block[j]->instruction_.target_ != function_in.labels_[target_label].target_
											&& block[j]->instruction_.target_ != function_in.labels_[extended_target_label].target_
											&& (block[j]->instruction_.target_ > id
												|| block[j]->instruction_.target_ <= block[j]->instruction_.id_))
										{
											break;
										}
									default:
										continue;
									}

									is_possible_condition = false;
									break;
								}

								if (is_possible_condition) {
									for (std::uint32_t j = condition_blocks.size(); is_possible_condition && j--;) {
										for (std::uint32_t k = condition_blocks[j].size(); k--;) {
											if (!function_in.is_valid_label(condition_blocks[j][k]->instruction_.label_)
												|| ((function_in.labels_[condition_blocks[j][k]->instruction_.label_].jump_ids_.front() >= condition_blocks[j].front()->instruction_.id_
														|| !k
														|| (has_bool_construct
															&& condition_blocks[j][k - 1]->type_ == statement::type::t_condition
															&& (condition_blocks[j][k - 1]->instruction_.target_ == block[i]->instruction_.id_
																|| condition_blocks[j][k - 1]->instruction_.target_ == block[i - 2]->instruction_.id_)))
													&& function_in.labels_[condition_blocks[j][k]->instruction_.label_].jump_ids_.back() < condition_blocks[j][k]->instruction_.id_))
											{
												continue;
											}
											is_possible_condition = false;
											break;
										}
									}

									if (is_possible_condition) {
										target_slot_scope = function_in.slot_scope_collector_.slot_infos_[target_slot].active_slot_scope_;
										function_in.slot_scope_collector_.slot_infos_[target_slot].min_scope_begin_ = INVALID_ID;
										i++;

										for (std::uint32_t j = 0; j < condition_blocks.size(); j++) {
											if (j
												&& (!has_bool_construct
													|| j != 2
													|| condition_blocks[j].back()->type_ != statement::type::t_condition)) {
												(*target_slot_scope)->usages_++;
												function_in.slot_scope_collector_.slot_infos_[target_slot].active_slot_scope_ = target_slot_scope;
											}

											this->build_slot_scopes(function_in, condition_blocks[j], nullptr);
											i -= condition_blocks[j].size();
											if (!function_in.slot_scope_collector_.slot_infos_[target_slot].active_slot_scope_ || j == condition_blocks.size() - 1) {
												continue;
											}

											while (function_in.slot_scope_collector_.slot_infos_[target_slot].slot_scopes_.back() != target_slot_scope) {
												(*target_slot_scope)->usages_ += (*function_in.slot_scope_collector_.slot_infos_[target_slot].slot_scopes_.back())->usages_ + 1;

												for (std::uint32_t k = (*function_in.slot_scope_collector_.slot_infos_[target_slot].slot_scopes_.back())->merged_scopes_.size(); k--;) {
													*(*function_in.slot_scope_collector_.slot_infos_[target_slot].slot_scopes_.back())->merged_scopes_[k] = *target_slot_scope;
												}

												(*target_slot_scope)->merged_scopes_.insert((*target_slot_scope)->merged_scopes_.begin(),
													(*function_in.slot_scope_collector_.slot_infos_[target_slot].slot_scopes_.back())->merged_scopes_.begin(),
													(*function_in.slot_scope_collector_.slot_infos_[target_slot].slot_scopes_.back())->merged_scopes_.end());
												(*target_slot_scope)->merged_scopes_.emplace_back(&(*function_in.slot_scope_collector_.slot_infos_[target_slot].slot_scopes_.back())->slot_scope_);
												*function_in.slot_scope_collector_.slot_infos_[target_slot].slot_scopes_.back() = *target_slot_scope;
												function_in.slot_scope_collector_.slot_infos_[target_slot].slot_scopes_.pop_back();
											}

											function_in.slot_scope_collector_.slot_infos_[target_slot].active_slot_scope_ = target_slot_scope;
											function_in.slot_scope_collector_.extend_scope(target_slot, function_in.get_scope_begin_from_label(target_label, (*target_slot_scope)->scope_end_));
											break;
										}

										continue;
									}
								}
							}
						}
					}
				}
			}
			else {
				id = function_in.slot_scope_collector_.previous_id_ - 1;
			}

			function_in.slot_scope_collector_.begin_upvalue_scopes(id);

			if (block[i]->function_) {
				for (auto j = block[i]->function_->upvalues_.size(); j--;) {
					if (!block[i]->function_->upvalues_[j].local_) {
						continue;
					}
					if (block[i]->function_->upvalues_[j].slot_ == block[i]->assignment_.variables_.back().slot_) {
						block[i]->function_->assignment_slot_is_upvalue_ = true;
					}
					function_in.slot_scope_collector_.add_to_scope(block[i]->function_->upvalues_[j].slot_, block[i]->function_->upvalues_[j].slot_scope_, id);
				}
			}

			for (std::uint8_t j = block[i]->assignment_.variables_.size(); j--;) {
				switch (block[i]->assignment_.variables_[j].type_) {
				case variable::type::t_slot:
					if (block[i]->type_ != statement::type::t_declaration || function_in.slot_scope_collector_.slot_infos_[block[i]->assignment_.variables_[j].slot_].min_scope_begin_ >= id) {
						function_in.slot_scope_collector_.close_scope(block[i]->assignment_.variables_[j].slot_, block[i]->assignment_.variables_[j].slot_scope_, id);
						continue;
					}
					
					index = function_in.slot_scope_collector_.slot_infos_[block[i]->assignment_.variables_[j].slot_].min_scope_begin_;
					function_in.slot_scope_collector_.slot_infos_[block[i]->assignment_.variables_[j].slot_].min_scope_begin_ = INVALID_ID;
					function_in.slot_scope_collector_.close_scope(block[i]->assignment_.variables_[j].slot_, block[i]->assignment_.variables_[j].slot_scope_, id);
					function_in.slot_scope_collector_.slot_infos_[block[i]->assignment_.variables_[j].slot_].min_scope_begin_ = index;
					continue;
				case variable::type::t_table_index:
					function_in.slot_scope_collector_.add_to_scope(block[i]->assignment_.variables_[j].table_->variable_->slot_,
						block[i]->assignment_.variables_[j].table_->variable_->slot_scope_, id);
					continue;
				default:
					break;
				}
			}

			ld_assert(!block[i]->assignment_.variables_.size()
				|| block[i]->assignment_.variables_.front().type_ != variable::type::t_slot
				|| !block[i]->assignment_.variables_.front().is_multres_
				|| ((*block[i]->assignment_.variables_.front().slot_scope_)->usages_ == 1
					&& (!function_in.slot_scope_collector_.slot_infos_[block[i]->assignment_.variables_.front().slot_].active_slot_scope_
						|| *function_in.slot_scope_collector_.slot_infos_[block[i]->assignment_.variables_.front().slot_].active_slot_scope_ != *block[i]->assignment_.variables_.front().slot_scope_)),
				"Multres assignment has invalid number of usages", this->bc_.file_path_);

			for (std::uint8_t j = block[i]->assignment_.open_slots_.size(); j--;) {
				function_in.slot_scope_collector_.add_to_scope((*block[i]->assignment_.open_slots_[j])->variable_->slot_,
					(*block[i]->assignment_.open_slots_[j])->variable_->slot_scope_, id);
			}

			if (block[i]->instruction_.id_ != INVALID_ID) {
				function_in.slot_scope_collector_.previous_id_ = id;

				if (function_in.is_valid_label(block[i]->instruction_.label_)) {
					id = function_in.get_scope_end_from_label(block[i]->instruction_.label_);
					if (id > block[i]->instruction_.id_) {
						function_in.slot_scope_collector_.merge_scopes(id);
					}
					function_in.slot_scope_collector_.extend_scopes(function_in.get_scope_begin_from_label(block[i]->instruction_.label_, id));
				}
			}
		}
	}

	void ast::eliminate_slots(function& function_in, std::vector<statement*>& block, block_info* const& previous_block) {
		static bool (*const has_self_reference)(const std::uint8_t&, expression* const&) = [](const std::uint8_t& targetSlot, expression* const& expr)->bool {
			switch (expr->type_) {
			case expression::type::t_function:
				for (std::uint8_t i = expr->function_->upvalues_.size(); i--;) {
					if (expr->function_->upvalues_[i].local_ && expr->function_->upvalues_[i].slot_ == targetSlot) {
						return true;
					}
				}
				break;
			case expression::type::t_variable:
				switch (expr->variable_->type_) {
				case variable::type::t_slot:
					return expr->variable_->slot_ == targetSlot;
				case variable::type::t_table_index:
					return has_self_reference(targetSlot, expr->variable_->table_) || has_self_reference(targetSlot, expr->variable_->table_index_);
				default:
					break;
				}
				break;
			case expression::type::t_function_call:
				if (has_self_reference(targetSlot, expr->function_call_->function_)) {
					return true;
				}
				for (std::uint32_t i = expr->function_call_->arguments_.size(); i--;) {
					if (has_self_reference(targetSlot, expr->function_call_->arguments_[i])) {
						return true;
					}
				}
				if (expr->function_call_->multres_argument_) {
					return has_self_reference(targetSlot, expr->function_call_->multres_argument_);
				}
				break;
			case expression::type::t_table:
				for (std::uint32_t i = expr->table_->fields_.size(); i--;) {
					if (has_self_reference(targetSlot, expr->table_->fields_[i].key_) || has_self_reference(targetSlot, expr->table_->fields_[i].value_)) {
						return true;
					}
				}
				if (expr->table_->multres_field_) {
					return has_self_reference(targetSlot, expr->table_->multres_field_);
				}
				break;
			case expression::type::t_binary_operation:
				return has_self_reference(targetSlot, expr->binary_operation_->left_operand_) || has_self_reference(targetSlot, expr->binary_operation_->right_operand_);
			case expression::type::t_unary_operation:
				return has_self_reference(targetSlot, expr->unary_operation_->operand_);
			default:
				break;
			}
			return false;
		};

		block_info info = {
			.block_ = block,
			.previous_block_ = previous_block
		};
		expression* expr;
		std::uint32_t index, target_index, target_label, extended_target_label;
		bool has_bool_construct;

		for (std::uint32_t i = 0; i < block.size(); i++) {
			switch (block[i]->type_) {
			case statement::type::t_condition:
				if (block[i]->condition_.allow_slot_swap_
					&& i
					&& !function_in.is_valid_label(block[i]->instruction_.label_)
					&& block[i - 1]->type_ == statement::type::t_assignment
					&& block[i - 1]->assignment_.variables_.size() == 1
					&& block[i - 1]->assignment_.variables_.back().type_ == variable::type::t_slot
					&& (*block[i - 1]->assignment_.variables_.back().slot_scope_)->usages_ == 1
					&& block[i - 1]->assignment_.variables_.back().slot_ == block[i]->assignment_.expressions_[0]->variable_->slot_) {
					expr = block[i]->assignment_.expressions_[0];
					block[i]->assignment_.expressions_[0] = block[i]->assignment_.expressions_[1];
					block[i]->assignment_.expressions_[1] = expr;
					block[i]->condition_.swapped_ = true;
				}

				break;
			case statement::type::t_generic_for:
			case statement::type::t_declaration:
				while (i && !function_in.is_valid_label(block[i]->instruction_.label_)) {
					switch (block[i - 1]->type_) {
					case statement::type::t_assignment:
						if (block[i - 1]->assignment_.variables_.front().slot_ <= block[i]->assignment_.expressions_[block[i]->assignment_.open_slots_.size() - 1]->variable_->slot_) {
							break;
						}
						ld_assert(block[i - 1]->assignment_.variables_.size() == 1 && !(*block[i - 1]->assignment_.variables_.back().slot_scope_)->usages_,
							"Invalid expression list assignment", this->bc_.file_path_);
					case statement::type::t_function_call:
						block[i]->assignment_.expressions_.emplace(block[i]->assignment_.expressions_.begin() + block[i]->assignment_.open_slots_.size(),
							block[i - 1]->assignment_.expressions_.back());
						block[i]->instruction_.label_ = block[i - 1]->instruction_.label_;
						i--;
						block.erase(block.begin() + i);
						continue;
					default:
						break;
					}
					if (block[i - 1]->type_ == statement::type::t_assignment && block[i - 1]->assignment_.variables_.size() != 1) {
						ld_assert(block[i]->assignment_.expressions_.size() == block[i]->assignment_.open_slots_.size()
							&& block[i]->assignment_.expressions_.back()->variable_->slot_ == block[i - 1]->assignment_.variables_.back().slot_,
							"Invalid multres expression list assignment", this->bc_.file_path_);

						while (true) {
							function_in.slot_scope_collector_.remove_scope(block[i]->assignment_.expressions_.back()->variable_->slot_,
								block[i]->assignment_.expressions_.back()->variable_->slot_scope_);
							block[i]->assignment_.open_slots_.pop_back();

							if (block[i]->assignment_.expressions_.back()->variable_->slot_ != block[i - 1]->assignment_.variables_.front().slot_) {
								block[i]->assignment_.expressions_.pop_back();
								continue;
							}

							block[i]->assignment_.expressions_.back() = block[i - 1]->assignment_.expressions_.back();
							block[i]->instruction_.label_ = block[i - 1]->instruction_.label_;
							i--;
							block.erase(block.begin() + i);
							break;
						}
					}
					for (std::uint32_t j = block[i]->assignment_.open_slots_.size(); j--;) {
						block[i]->assignment_.open_slots_[j] = &block[i]->assignment_.expressions_[j];
					}
					break;
				}
				break;
			case statement::type::t_assignment:
				switch (block[i]->assignment_.variables_.back().type_) {
				case variable::type::t_slot:
					if (block[i]->assignment_.expressions_.back()->type_ == expression::type::t_binary_operation
						&& block[i]->assignment_.expressions_.back()->binary_operation_->type_ != binary_operation::type::t_concatenation
						&& block[i]->assignment_.open_slots_.size() == 2
						&& i >= 2
						&& !function_in.is_valid_label(block[i]->instruction_.label_)
						&& !function_in.is_valid_label(block[i - 1]->instruction_.label_)
						&& block[i - 1]->type_ == statement::type::t_assignment
						&& block[i - 1]->assignment_.variables_.size() == 1
						&& block[i - 1]->assignment_.variables_.back().type_ == variable::type::t_slot
						&& (*block[i - 1]->assignment_.variables_.back().slot_scope_)->usages_ == 1
						&& block[i - 1]->assignment_.variables_.back().slot_ == block[i]->assignment_.expressions_.back()->binary_operation_->left_operand_->variable_->slot_
						&& get_constant_type(block[i - 1]->assignment_.expressions_.back()) == constant_type::t_number
						&& block[i - 2]->type_ == statement::type::t_assignment
						&& block[i - 2]->assignment_.variables_.size() == 1
						&& block[i - 2]->assignment_.variables_.back().type_ == variable::type::t_slot
						&& (*block[i - 2]->assignment_.variables_.back().slot_scope_)->usages_ == 1
						&& block[i - 2]->assignment_.variables_.back().slot_ == block[i]->assignment_.expressions_.back()->binary_operation_->right_operand_->variable_->slot_)
					{
						block[i]->assignment_.open_slots_[0] = &block[i]->assignment_.expressions_.back()->binary_operation_->right_operand_;
						block[i]->assignment_.open_slots_[1] = &block[i]->assignment_.expressions_.back()->binary_operation_->left_operand_;
					}
					break;
				case variable::type::t_table_index:
					if (!block[i]->assignment_.variables_.back().is_multres_
						&& block[i]->assignment_.variables_.back().table_index_->type_ == expression::type::t_variable
						&& i >= 3
						&& !function_in.is_valid_label(block[i]->instruction_.label_)
						&& !function_in.is_valid_label(block[i - 1]->instruction_.label_)
						&& !function_in.is_valid_label(block[i - 2]->instruction_.label_)
						&& block[i - 1]->type_ == statement::type::t_assignment
						&& block[i - 1]->assignment_.variables_.size() == 1
						&& block[i - 1]->assignment_.variables_.back().type_ == variable::type::t_slot
						&& (*block[i - 1]->assignment_.variables_.back().slot_scope_)->usages_ == 1
						&& block[i - 1]->assignment_.variables_.back().slot_ == block[i]->assignment_.variables_.back().table_index_->variable_->slot_
						&& get_constant_type(block[i - 1]->assignment_.expressions_.back()) != constant_type::t_invalid
						&& block[i - 2]->type_ == statement::type::t_assignment
						&& block[i - 2]->assignment_.variables_.size() == 1
						&& block[i - 2]->assignment_.variables_.back().type_ == variable::type::t_slot
						&& (*block[i - 2]->assignment_.variables_.back().slot_scope_)->usages_ == 1
						&& block[i - 2]->assignment_.variables_.back().slot_ == block[i]->assignment_.expressions_.back()->variable_->slot_
						&& (get_constant_type(block[i - 2]->assignment_.expressions_.back()) == constant_type::t_invalid
							|| get_constant_type(block[i - 1]->assignment_.expressions_.back()) == constant_type::t_nil)
						&& block[i - 3]->assignment_.is_table_constructor_
						&& block[i - 3]->assignment_.variables_.back().slot_ == block[i]->assignment_.variables_.back().table_->variable_->slot_
						&& !block[i - 3]->assignment_.expressions_.back()->table_->multres_field_)
					{
						block[i]->assignment_.open_slots_[0] = &block[i]->assignment_.expressions_.back();
						block[i]->assignment_.open_slots_[1] = &block[i]->assignment_.variables_.back().table_index_;
					}
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
			if (block[i]->type_ == statement::type::t_declaration
				&& block[i]->assignment_.open_slots_.size() == 1
				&& (*(*block[i]->assignment_.open_slots_.back())->variable_->slot_scope_)->usages_ > 1
				&& i
				&& block[i - 1]->type_ == statement::type::t_assignment
				&& block[i - 1]->assignment_.variables_.size() == 1
				&& block[i - 1]->assignment_.variables_.back().type_ == variable::type::t_slot
				&& block[i - 1]->function_
				&& block[i - 1]->function_->assignment_slot_is_upvalue_
				&& block[i - 1]->assignment_.variables_.back().slot_ == (*block[i]->assignment_.open_slots_.back())->variable_->slot_)
			{
				*block[i]->assignment_.open_slots_.back() = block[i - 1]->assignment_.expressions_.back();
				*block[i - 1]->assignment_.variables_.back().slot_scope_ = *block[i]->assignment_.variables_.back().slot_scope_;
				block[i]->instruction_.label_ = block[i - 1]->instruction_.label_;
				i--;
				function_in.slot_scope_collector_.remove_scope(block[i]->assignment_.variables_.back().slot_,
					block[i]->assignment_.variables_.back().slot_scope_);
				block.erase(block.begin() + i);
			}
			else {
				for (std::uint8_t j = block[i]->assignment_.open_slots_.size();
					j--
					&& i
					&& !function_in.is_valid_label(block[i]->instruction_.label_)
					&& block[i - 1]->type_ == statement::type::t_assignment
					&& block[i - 1]->assignment_.variables_.size() == 1
					&& block[i - 1]->assignment_.variables_.back().type_ == variable::type::t_slot
					&& (*block[i - 1]->assignment_.variables_.back().slot_scope_)->usages_ == 1;)
				{
					if (j == 1
						&& block[i]->assignment_.is_potential_method_
						&& i >= 2
						&& !function_in.is_valid_label(block[i - 1]->instruction_.label_)
						&& block[i - 1]->assignment_.variables_.back().slot_ == (*block[i]->assignment_.open_slots_.front())->variable_->slot_
						&& block[i - 1]->assignment_.expressions_.back()->type_ == expression::type::t_variable
						&& block[i - 1]->assignment_.expressions_.back()->variable_->type_ == variable::type::t_table_index
						&& block[i - 1]->assignment_.expressions_.back()->variable_->table_->type_ == expression::type::t_variable
						&& block[i - 1]->assignment_.expressions_.back()->variable_->table_->variable_->type_ == variable::type::t_slot
						&& block[i - 1]->assignment_.expressions_.back()->variable_->table_index_->type_ == expression::type::t_constant
						&& block[i - 1]->assignment_.expressions_.back()->variable_->table_index_->constant_->is_name_
						&& block[i - 2]->type_ == statement::type::t_assignment
						&& block[i - 2]->assignment_.variables_.size() == 1
						&& block[i - 2]->assignment_.variables_.back().type_ == variable::type::t_slot
						&& (*block[i - 2]->assignment_.variables_.back().slot_scope_)->usages_ == 1
						&& block[i - 2]->assignment_.variables_.back().slot_ == (*block[i]->assignment_.open_slots_[j])->variable_->slot_
						&& block[i - 2]->assignment_.expressions_.back()->type_ == expression::type::t_variable
						&& block[i - 2]->assignment_.expressions_.back()->variable_->type_ == variable::type::t_slot
						&& block[i - 2]->assignment_.expressions_.back()->variable_->slot_ == block[i - 1]->assignment_.expressions_.back()->variable_->table_->variable_->slot_)
					{
						if (block[i]->type_ == statement::type::t_return) {
							block[i]->assignment_.multres_return_->function_call_->is_method_ = true;
							block[i]->assignment_.multres_return_->function_call_->arguments_.erase(block[i]->assignment_.multres_return_->function_call_->arguments_.begin());
						}
						else {
							block[i]->assignment_.expressions_.back()->function_call_->is_method_ = true;
							block[i]->assignment_.expressions_.back()->function_call_->arguments_.erase(block[i]->assignment_.expressions_.back()->function_call_->arguments_.begin());
						}

						block[i]->assignment_.open_slots_.erase(block[i]->assignment_.open_slots_.begin() + j);
						block[i]->assignment_.open_slots_.emplace(block[i]->assignment_.open_slots_.begin(), &block[i - 1]->assignment_.expressions_.back()->variable_->table_);
						function_in.slot_scope_collector_.remove_scope(block[i - 2]->assignment_.variables_.back().slot_, block[i - 2]->assignment_.variables_.back().slot_scope_);
						block[i - 1]->instruction_.label_ = block[i - 2]->instruction_.label_;
						(*block[i - 2]->assignment_.expressions_.back()->variable_->slot_scope_)->usages_--;
						i--;
						block.erase(block.begin() + i - 1);
					}
					if (block[i - 1]->assignment_.variables_.back().slot_ != (*block[i]->assignment_.open_slots_[j])->variable_->slot_) {
						continue;
					}
					ld_assert(block[i - 1]->assignment_.variables_.back().is_multres_ == (*block[i]->assignment_.open_slots_[j])->variable_->is_multres_,
						"Multres type mismatch when trying to eliminate slot", this->bc_.file_path_);
					expr = *block[i]->assignment_.open_slots_[j];
					*block[i]->assignment_.open_slots_[j] = block[i - 1]->assignment_.expressions_.back();
					if (!j
						&& block[i]->assignment_.allowed_constant_type_ != constant_type::t_number
						&& get_constant_type(block[i]->assignment_.expressions_.back()) > block[i]->assignment_.allowed_constant_type_)
					{
						*block[i]->assignment_.open_slots_[j] = expr;
						break;
					}
					function_in.slot_scope_collector_.remove_scope(block[i - 1]->assignment_.variables_.back().slot_, block[i - 1]->assignment_.variables_.back().slot_scope_);
					block[i]->instruction_.label_ = block[i - 1]->instruction_.label_;
					i--;
					block.erase(block.begin() + i);
				}
			}

			ld_assert(!block[i]->assignment_.open_slots_.size()
				|| (*block[i]->assignment_.open_slots_.back())->type_ != expression::type::t_variable
				|| !(*block[i]->assignment_.open_slots_.back())->variable_->is_multres_,
				"Unable to eliminate multres slot", this->bc_.file_path_);

			switch (block[i]->type_) {
			case statement::type::t_numeric_for:
			case statement::type::t_generic_for:
				this->eliminate_slots(function_in, block[i]->block_, nullptr);
				break;
			case statement::type::t_loop:
			case statement::type::t_declaration:
				info.index_ = i;
				this->eliminate_slots(function_in, block[i]->block_, &info);
				break;
			case statement::type::t_assignment:
				if (block[i]->assignment_.variables_.size() == 1) {
					switch (block[i]->assignment_.variables_.back().type_) {
					case variable::type::t_slot:
						if (block[i]->instruction_.id_ == INVALID_ID) {
							break;
						}
						info.index_ = i;
						target_label = this->get_label_from_next_statement(function_in, info, false, true);
						extended_target_label = this->get_label_from_next_statement(function_in, info, true, true);
						if (!function_in.is_valid_label(target_label) || function_in.labels_[target_label].jump_ids_.front() > block[i]->instruction_.id_) {
							break;
						}
						if ((*block[i]->assignment_.variables_.back().slot_scope_)->usages_ >= 2) {
							if ((*block[i]->assignment_.variables_.back().slot_scope_)->scope_begin_ >= function_in.labels_[target_label].jump_ids_.front()
								|| (extended_target_label != target_label
									&& (function_in.labels_[extended_target_label].target_ <= block[i]->instruction_.id_
										|| function_in.labels_[extended_target_label].target_ >= function_in.labels_[target_label].jump_ids_.front()))
								|| has_self_reference(block[i]->assignment_.variables_.back().slot_, block[i]->assignment_.expressions_.back()))
								break;
							index = this->get_block_index_from_id(block, function_in.labels_[target_label].jump_ids_.front() - 1);
							if (index == INVALID_ID) {
								break;
							}
							switch (block[index]->type_) {
							case statement::type::t_condition:
								if (block[index]->assignment_.variables_.size()) {
									if ((*block[index]->assignment_.variables_.back().slot_scope_)->scope_begin_ == block[index]->instruction_.id_
										&& *block[index]->assignment_.variables_.back().slot_scope_ == *block[i]->assignment_.variables_.back().slot_scope_)
										break;
								}
								else if (index
										&& block[index]->assignment_.expressions_.size() == 1
										&& !function_in.is_valid_label(block[index]->instruction_.label_)
										&& block[index - 1]->type_ == statement::type::t_assignment
										&& block[index - 1]->assignment_.variables_.size() == 1
										&& block[index - 1]->assignment_.variables_.back().type_ == variable::type::t_slot
										&& (*block[index - 1]->assignment_.variables_.back().slot_scope_)->scope_begin_ == block[index - 1]->instruction_.id_
										&& *block[index - 1]->assignment_.variables_.back().slot_scope_ == *block[i]->assignment_.variables_.back().slot_scope_)
								{
									break;
								}

								index = INVALID_ID;
								break;
							case statement::type::t_assignment:
								if (block[index]->assignment_.variables_.size() != 1
									|| block[index]->assignment_.variables_.back().type_ != variable::type::t_slot
									|| (*block[index]->assignment_.variables_.back().slot_scope_)->scope_begin_ != block[index]->instruction_.id_
									|| *block[index]->assignment_.variables_.back().slot_scope_ != *block[i]->assignment_.variables_.back().slot_scope_
									|| (index != i - 4
										&& (block[index]->assignment_.expressions_.back()->type_ != expression::type::t_constant
											|| get_constant_type(block[index]->assignment_.expressions_.back()) == constant_type::t_invalid)))
									index = INVALID_ID;
								break;
							default:
								break;
							}

							if (index == INVALID_ID) {
								break;
							}
							has_bool_construct = false;

							if (i >= 3
								&& block[i]->type_ == statement::type::t_assignment
								&& block[i]->assignment_.expressions_.back()->type_ == expression::type::t_constant
								&& block[i]->assignment_.expressions_.back()->constant_->type_ == constant::type::t_true
								&& (block[i - 1]->type_ == statement::type::t_goto
									|| block[i - 1]->type_ == statement::type::t_break)
								&& !function_in.is_valid_label(block[i - 1]->instruction_.label_)
								&& block[i - 1]->instruction_.type_ == bytecode::opcode::op_jmp
								&& block[i - 1]->instruction_.target_ == function_in.labels_[target_label].target_
								&& block[i - 2]->type_ == statement::type::t_assignment
								&& block[i - 2]->assignment_.expressions_.back()->type_ == expression::type::t_constant
								&& block[i - 2]->assignment_.expressions_.back()->constant_->type_ == constant::type::t_false
								&& block[i - 2]->assignment_.variables_.size() == 1
								&& block[i - 2]->assignment_.variables_.back().type_ == variable::type::t_slot
								&& *block[i - 2]->assignment_.variables_.back().slot_scope_ == *block[i]->assignment_.variables_.back().slot_scope_)
							{
								switch (block[i - 3]->type_) {
								case statement::type::t_condition:
									if (block[i - 3]->assignment_.expressions_.size() == 2 && block[i - 3]->instruction_.target_ == block[i]->instruction_.id_) {
										has_bool_construct = true;
									}
									break;
								case statement::type::t_goto:
								case statement::type::t_break:
									if (i < 5
										|| function_in.is_valid_label(block[i - 3]->instruction_.label_)
										|| block[i - 3]->instruction_.type_ != bytecode::opcode::op_jmp
										|| block[i - 3]->instruction_.target_ != function_in.labels_[extended_target_label].target_
										|| (!function_in.is_valid_label(block[i]->instruction_.label_)
											&& !function_in.is_valid_label(block[i - 2]->instruction_.label_))
										|| block[i - 4]->type_ != statement::type::t_assignment
										|| block[i - 4]->assignment_.variables_.size() != 1
										|| block[i - 4]->assignment_.variables_.back().type_ != variable::type::t_slot
										|| block[i - 4]->assignment_.variables_.back().slot_ != block[i]->assignment_.variables_.back().slot_)
									{
										break;
									}
									if (index == i - 2 && !function_in.is_valid_label(block[i]->instruction_.label_)) {
										if (function_in.labels_[block[i - 2]->instruction_.label_].jump_ids_.front() > block[i - 2]->instruction_.id_) {
											break;
										}
										index = this->get_block_index_from_id(block, function_in.labels_[block[i - 2]->instruction_.label_].jump_ids_.front() - 1);
										if (index == INVALID_ID) {
											index = i - 2;
											break;
										}
									}

									has_bool_construct = true;
									break;
								default:
									break;
								}

								if (has_bool_construct) {
									if ((function_in.is_valid_label(block[i]->instruction_.label_)
										&& function_in.labels_[block[i]->instruction_.label_].jump_ids_.back() >= block[i]->instruction_.id_)
										|| (function_in.is_valid_label(block[i - 2]->instruction_.label_)
											&& function_in.labels_[block[i - 2]->instruction_.label_].jump_ids_.back() >= block[i - 2]->instruction_.id_))
									{
										break;
									}

									if (function_in.is_valid_label(block[i]->instruction_.label_)) {
										for (std::uint32_t j = function_in.labels_[block[i]->instruction_.label_].jump_ids_.size(); j--;) {
											target_index = this->get_block_index_from_id(block, function_in.labels_[block[i]->instruction_.label_].jump_ids_[j] - 1);
											if (target_index == INVALID_ID
												|| block[target_index]->type_ != statement::type::t_condition
												|| block[target_index]->assignment_.variables_.size())
											{
												index = INVALID_ID;
												break;
											}
											if (!block[target_index]->assignment_.expressions_.size()) {
												has_bool_construct = false;
												break;
											}
										}
									}

									if (has_bool_construct && function_in.is_valid_label(block[i - 2]->instruction_.label_)) {
										for (std::uint32_t j = function_in.labels_[block[i - 2]->instruction_.label_].jump_ids_.size(); j--;) {
											target_index = get_block_index_from_id(block, function_in.labels_[block[i - 2]->instruction_.label_].jump_ids_[j] - 1);

											if (target_index == INVALID_ID || block[target_index]->type_ != statement::type::t_condition) {
												index = INVALID_ID;
												break;
											}

											if (!block[target_index]->assignment_.expressions_.size() || block[target_index]->assignment_.variables_.size()) {
												has_bool_construct = false;
												break;
											}
										}
									}

									if (index == INVALID_ID) break;
								}
							}

							for (std::uint32_t j = i; index != INVALID_ID && block[index]->instruction_.id_ < block[j]->instruction_.id_; j--) {
								if (function_in.is_valid_label(block[j]->instruction_.label_)) {
									if (function_in.labels_[block[j]->instruction_.label_].jump_ids_.back() >= block[j]->instruction_.id_) {
										index = INVALID_ID;
										break;
									}
									while (function_in.labels_[block[j]->instruction_.label_].jump_ids_.front() < block[index]->instruction_.id_) {
										if (!index) {
											index = INVALID_ID;
											break;
										}
										index--;
									}
								}
							}

							if (index != INVALID_ID) {
								switch (block[index]->type_) {
								case statement::type::t_condition:
									if (block[index]->assignment_.variables_.size()) {
										break;
									}
								case statement::type::t_goto:
								case statement::type::t_break:
									if (block[index]->instruction_.target_ == function_in.labels_[target_label].target_ && index) {
										index--;
									}
								default:
									break;
								}

								condition_builder builder(condition_builder::type::t_assignment, *this, target_label,
									has_bool_construct ? block[i]->instruction_.label_ : INVALID_ID, has_bool_construct ? block[i - 2]->instruction_.label_ : INVALID_ID);
								target_index = has_bool_construct ? (block[i - 3]->type_ == statement::type::t_goto ? i - 4 : i - 2) : i;

								for (std::uint32_t j = index; j < target_index; j++) {
									switch (block[j]->type_) {
									case statement::type::t_condition:
										if (block[j]->instruction_.target_ <= block[j]->instruction_.id_
											|| block[j]->instruction_.target_ > function_in.labels_[target_label].target_
											|| (block[j]->instruction_.target_ == function_in.labels_[target_label].target_
												? !block[j]->assignment_.variables_.size()
													|| *block[j]->assignment_.variables_.back().slot_scope_ != *block[i]->assignment_.variables_.back().slot_scope_
													|| has_self_reference(block[i]->assignment_.variables_.back().slot_, block[j]->assignment_.expressions_.back())
												: block[j]->assignment_.variables_.size()))
										{
											break;
										}
										builder.add_node(builder.get_node_type(block[j]->instruction_.type_, block[j]->condition_.swapped_),
											block[j]->instruction_.label_, function_in.get_label_from_id(block[j]->instruction_.target_),
											&block[j]->assignment_.expressions_);
										continue;
									case statement::type::t_assignment:
										if (block[j]->assignment_.variables_.size() != 1
											|| block[j]->assignment_.variables_.back().type_ != variable::type::t_slot
											|| *block[j]->assignment_.variables_.back().slot_scope_ != *block[i]->assignment_.variables_.back().slot_scope_
											|| has_self_reference(block[i]->assignment_.variables_.back().slot_, block[j]->assignment_.expressions_.back())
											|| j + 1 == target_index
											|| function_in.is_valid_label(block[j + 1]->instruction_.label_))
											break;
										j++;

										switch (block[j]->type_) {
										case statement::type::t_condition:
											if (block[j]->instruction_.target_ != function_in.labels_[target_label].target_
												|| block[j]->assignment_.variables_.size()
												|| block[j]->assignment_.expressions_.size() != 1
												|| block[j]->assignment_.expressions_.back()->type_ != expression::type::t_variable
												|| block[j]->assignment_.expressions_.back()->variable_->type_ != variable::type::t_slot
												|| *block[j]->assignment_.expressions_.back()->variable_->slot_scope_ != *block[i]->assignment_.variables_.back().slot_scope_)
											{
												break;
											}
											builder.add_node(builder.get_node_type(block[j]->instruction_.type_, block[j]->condition_.swapped_),
												block[j - 1]->instruction_.label_,
												function_in.get_label_from_id(block[j]->instruction_.target_), &block[j - 1]->assignment_.expressions_);
											continue;
										case statement::type::t_goto:
										case statement::type::t_break:
											if (function_in.is_valid_label(block[j]->instruction_.label_)
												|| block[j]->instruction_.type_ != bytecode::opcode::op_jmp
												|| block[j]->instruction_.target_ != function_in.labels_[target_label].target_
												|| block[j - 1]->assignment_.expressions_.back()->type_ != expression::type::t_constant
												|| get_constant_type(block[j - 1]->assignment_.expressions_.back()) == constant_type::t_invalid)
											{
												break;
											}
											switch (block[j - 1]->assignment_.expressions_.back()->constant_->type_) {
											case constant::type::t_nil:
											case constant::type::t_false:
												builder.add_node(condition_builder::node::type::t_falsy_test, block[j - 1]->instruction_.label_,
													function_in.get_label_from_id(block[j]->instruction_.target_), &block[j - 1]->assignment_.expressions_);
												break;
											case constant::type::t_true:
											case constant::type::t_string:
											case constant::type::t_number:
												builder.add_node(condition_builder::node::type::t_truthy_test, block[j - 1]->instruction_.label_,
													function_in.get_label_from_id(block[j]->instruction_.target_), &block[j - 1]->assignment_.expressions_);
												break;
											default:
												break;
											}
											continue;
										default:
											break;
										}
										break;
									default:
										break;
									}

									index = INVALID_ID;
									break;
								}

								if (!has_bool_construct) {
									builder.add_node(condition_builder::node::type::t_truthy_test, block[i]->instruction_.label_, target_label,
										&block[i]->assignment_.expressions_);
								}
								else if (block[i - 3]->type_ == statement::type::t_goto) {
									builder.add_node(condition_builder::node::type::t_falsy_test, block[i - 4]->instruction_.label_, target_label,
										&block[i - 4]->assignment_.expressions_);
								}

								if (index != INVALID_ID) {
									expr = builder.build_condition();
									if (!expr) {
										break;
									}
									block[i]->assignment_.expressions_.back() = expr;

									for (std::uint32_t j = index; j < i; j++) {
										switch (block[j]->type_) {
										case statement::type::t_condition:
											if (block[j]->instruction_.target_ == function_in.labels_[target_label].target_) {
												(*block[i]->assignment_.variables_.back().slot_scope_)->usages_--;
											}
											function_in.remove_jump(block[j]->instruction_.id_ + 1, block[j]->instruction_.target_);
											if (block[j]->assignment_.variables_.size()) {
												function_in.remove_jump(block[j]->instruction_.id_, block[j]->instruction_.id_ + 2);
											}
											continue;
										case statement::type::t_goto:
										case statement::type::t_break:
											function_in.remove_jump(block[j]->instruction_.id_, block[j]->instruction_.target_);
											continue;
										case statement::type::t_assignment:
											(*block[i]->assignment_.variables_.back().slot_scope_)->usages_--;
											continue;
										default:
											break;
										}
									}

									block[i]->instruction_.label_ = block[index]->instruction_.label_;
									block[i]->assignment_.is_table_constructor_ = false;
									block.erase(block.begin() + index, block.begin() + i);
									i = index;
								}
							}
						}
						else {
							if ((*block[i]->assignment_.variables_.back().slot_scope_)->usages_ == 1
								&& (i == block.size() - 1 || block[i + 1]->type_ != statement::type::t_declaration))
							{
								break;
							}
							//TODO
						}
						break;
					case variable::type::t_table_index:
						if (i
							&& !function_in.is_valid_label(block[i]->instruction_.label_)
							&& block[i - 1]->type_ == statement::type::t_assignment
							&& block[i - 1]->assignment_.variables_.size() == 1
							&& block[i - 1]->assignment_.variables_.back().type_ == variable::type::t_slot
							&& block[i - 1]->assignment_.variables_.back().slot_ == block[i]->assignment_.variables_.back().table_->variable_->slot_) {
							if (block[i - 1]->assignment_.is_table_constructor_
								&& !block[i - 1]->assignment_.expressions_.back()->table_->multres_field_
								&& (block[i]->assignment_.variables_.back().is_multres_
									|| ENUM_UNDER(get_constant_type(block[i]->assignment_.variables_.back().table_index_)) <= ENUM_UNDER(constant_type::t_nil)
									|| get_constant_type(block[i]->assignment_.expressions_.back()) == constant_type::t_invalid)
								&& (block[i]->assignment_.variables_.back().is_multres_
									|| !has_self_reference(block[i - 1]->assignment_.variables_.back().slot_, block[i]->assignment_.variables_.back().table_index_))
								&& !has_self_reference(block[i - 1]->assignment_.variables_.back().slot_, block[i]->assignment_.expressions_.back())) {
								if (block[i]->assignment_.variables_.back().is_multres_) {
									block[i - 1]->assignment_.expressions_.back()->table_->multres_index_ = block[i]->assignment_.variables_.back().multres_index_;
									block[i - 1]->assignment_.expressions_.back()->table_->multres_field_ = block[i]->assignment_.expressions_.back();
								} else {
									if (block[i]->assignment_.variables_.back().table_index_->type_ == expression::type::t_constant && block[i]->assignment_.variables_.back().table_index_->constant_->type_ == constant::type::t_string) {
										for (std::uint32_t j = block[i - 1]->assignment_.expressions_.back()->table_->constants_.fields_.size(); j--;) {
											if (block[i - 1]->assignment_.expressions_.back()->table_->constants_.fields_[j].key_->constant_->type_ != constant::type::t_string
												|| block[i - 1]->assignment_.expressions_.back()->table_->constants_.fields_[j].key_->constant_->string_ != block[i]->assignment_.variables_.back().table_index_->constant_->string_)
												continue;
											if (block[i - 1]->assignment_.expressions_.back()->table_->constants_.fields_[j].value_->constant_->type_ == constant::type::t_nil)
												block[i - 1]->assignment_.expressions_.back()->table_->constants_.fields_.erase(block[i - 1]->assignment_.expressions_.back()->table_->constants_.fields_.begin() + j);
											break;
										}
									}

									block[i - 1]->assignment_.expressions_.back()->table_->fields_.emplace_back();
									block[i - 1]->assignment_.expressions_.back()->table_->fields_.back().key_ = block[i]->assignment_.variables_.back().table_index_;
									block[i - 1]->assignment_.expressions_.back()->table_->fields_.back().value_ = block[i]->assignment_.expressions_.back();
								}

								(*block[i - 1]->assignment_.variables_.back().slot_scope_)->usages_--;
								block.erase(block.begin() + i);
								i -= 2;
								break;
							}

							if (!block[i]->assignment_.variables_.back().is_multres_ && (*block[i - 1]->assignment_.variables_.back().slot_scope_)->usages_ == 1) {
								block[i]->assignment_.variables_.back().table_ = block[i - 1]->assignment_.expressions_.back();
								function_in.slot_scope_collector_.remove_scope(block[i - 1]->assignment_.variables_.back().slot_,
									block[i - 1]->assignment_.variables_.back().slot_scope_);
								block[i]->instruction_.label_ = block[i - 1]->instruction_.label_;
								i--;
								block.erase(block.begin() + i);
								break;
							}
						}
						ld_assert(!block[i]->assignment_.variables_.back().is_multres_, "Unable to eliminate multres table index", this->bc_.file_path_);
						break;
					default:
						break;
					}
				}
				break;
			default:
				break;
			}
		}
	}

	void ast::eliminate_conditions(function& function_in, std::vector<statement*>& block, block_info* const& previous_block) {
		block_info blockInfo = {
			.block_ = block,
			.previous_block_ = previous_block
		};
		std::vector<expression*> expressions(1);
		std::uint32_t index, target_index, previous_valid_index, assignment_index, target_label, extended_target_label;
		bool has_bool_construct, has_end_assignment;

		for (std::uint32_t i = block.size(); i--;) {
			if (block[i]->instruction_.id_ == INVALID_ID) {
				continue;
			}
			blockInfo.index_ = i;
			target_label = this->get_label_from_next_statement(function_in, blockInfo, false, false);
			extended_target_label = this->get_label_from_next_statement(function_in, blockInfo, true, false);
			if (!function_in.is_valid_label(target_label) || function_in.labels_[target_label].jump_ids_.front() > block[i]->instruction_.id_) {
				continue;
			}
			switch (block[i]->type_) {
			case statement::type::t_condition:
				index = INVALID_ID;
				for (std::uint32_t j = function_in.labels_[target_label].jump_ids_.size(); j--;) {
					if (function_in.labels_[target_label].jump_ids_[j] >= function_in.labels_[target_label].target_) {
						continue;
					}
					index = this->get_block_index_from_id(block, function_in.labels_[target_label].jump_ids_[j] - 1);
					if (index == INVALID_ID) {
						break;
					}
					switch (block[index]->type_) {
					case statement::type::t_condition:
						if (!block[index]->assignment_.variables_.size()) {
							index = INVALID_ID;
							if (target_label == extended_target_label
								|| (block[index]->assignment_.expressions_.size() == 1
									&& block[index]->assignment_.expressions_.back()->type_ == expression::type::t_variable
									&& block[index]->assignment_.expressions_.back()->variable_->type_ == variable::type::t_slot))
							{
								continue;
							}
						}
						break;
					case statement::type::t_assignment:
						if ((block[index + 1]->type_ == statement::type::t_goto || block[index + 1]->type_ == statement::type::t_break)
							&& block[index + 1]->instruction_.type_ == bytecode::opcode::op_jmp
							&& block[index]->assignment_.variables_.size() == 1
							&& block[index]->assignment_.variables_.back().type_ == variable::type::t_slot
							&& block[index]->assignment_.expressions_.back()->type_ == expression::type::t_constant
							&& get_constant_type(block[index]->assignment_.expressions_.back()) != constant_type::t_invalid)
						{
							break;
						}
					default:
						index = INVALID_ID;
					}
					break;
				}
				if (index == INVALID_ID) {
					continue;
				}
				assignment_index = index;
				break;
			case statement::type::t_goto:
			case statement::type::t_break:
				if (!i
					|| function_in.is_valid_label(block[i]->instruction_.label_)
					|| block[i]->instruction_.type_ != bytecode::opcode::op_jmp
					|| block[i]->instruction_.target_ != function_in.labels_[target_label].target_
					|| block[i - 1]->type_ != statement::type::t_assignment
					|| block[i - 1]->assignment_.variables_.size() != 1
					|| block[i - 1]->assignment_.variables_.back().type_ != variable::type::t_slot
					|| block[i - 1]->assignment_.expressions_.back()->type_ != expression::type::t_constant
					|| get_constant_type(block[i - 1]->assignment_.expressions_.back()) == constant_type::t_invalid)
				{
					continue;
				}
				assignment_index = i - 1;
				break;
			case statement::type::t_assignment:
				if (block[i]->assignment_.variables_.size() != 1 || block[i]->assignment_.variables_.back().type_ != variable::type::t_slot) {
					continue;
				}
				assignment_index = i;
				break;
			default:
				continue;
			}

			index = assignment_index;
			has_bool_construct = false;

			if (i >= 3
				&& block[i]->type_ == statement::type::t_assignment
				&& block[i]->assignment_.expressions_.back()->type_ == expression::type::t_constant
				&& block[i]->assignment_.expressions_.back()->constant_->type_ == constant::type::t_true
				&& (block[i - 1]->type_ == statement::type::t_goto || block[i - 1]->type_ == statement::type::t_break)
				&& !function_in.is_valid_label(block[i - 1]->instruction_.label_)
				&& block[i - 1]->instruction_.type_ == bytecode::opcode::op_jmp
				&& block[i - 1]->instruction_.target_ == function_in.labels_[target_label].target_
				&& block[i - 2]->type_ == statement::type::t_assignment
				&& block[i - 2]->assignment_.expressions_.back()->type_ == expression::type::t_constant
				&& block[i - 2]->assignment_.expressions_.back()->constant_->type_ == constant::type::t_false
				&& block[i - 2]->assignment_.variables_.size() == 1
				&& block[i - 2]->assignment_.variables_.back().type_ == variable::type::t_slot
				&& block[i - 2]->assignment_.variables_.back().slot_ == block[assignment_index]->assignment_.variables_.back().slot_)
			{
				switch (block[i - 3]->type_) {
				case statement::type::t_condition:
					if (block[i - 3]->assignment_.expressions_.size() == 2 && block[i - 3]->instruction_.target_ == block[i]->instruction_.id_) {
						has_bool_construct = true;
					}
					break;
				case statement::type::t_goto:
				case statement::type::t_break:
					if (i >= 4
						&& (!function_in.is_valid_label(block[i - 3]->instruction_.label_)
							|| (function_in.labels_[block[i - 3]->instruction_.label_].jump_ids_.size() == 1
								&& block[i - 4]->type_ == statement::type::t_condition
								&& block[i - 4]->assignment_.variables_.size()))
						&& block[i - 3]->instruction_.type_ == bytecode::opcode::op_jmp
						&& block[i - 3]->instruction_.target_ == function_in.labels_[extended_target_label].target_
						&& (function_in.is_valid_label(block[i]->instruction_.label_)
							|| function_in.is_valid_label(block[i - 2]->instruction_.label_)))
					{
						has_bool_construct = true;
					}
					break;
				default:
					break;
				}

				if (has_bool_construct) {
					if ((function_in.is_valid_label(block[i]->instruction_.label_)
						&& function_in.labels_[block[i]->instruction_.label_].jump_ids_.back() >= block[i]->instruction_.id_)
						|| (function_in.is_valid_label(block[i - 2]->instruction_.label_)
							&& function_in.labels_[block[i - 2]->instruction_.label_].jump_ids_.back() >= block[i - 2]->instruction_.id_))
					{
						continue;
					}

					if (function_in.is_valid_label(block[i]->instruction_.label_)) {
						for (std::uint32_t j = function_in.labels_[block[i]->instruction_.label_].jump_ids_.size(); j--;) {
							target_index = this->get_block_index_from_id(block, function_in.labels_[block[i]->instruction_.label_].jump_ids_[j] - 1);

							if (target_index == INVALID_ID
								|| block[target_index]->type_ != statement::type::t_condition
								|| block[target_index]->assignment_.variables_.size())
							{
								index = INVALID_ID;
								break;
							}

							if (!block[target_index]->assignment_.expressions_.size()) {
								has_bool_construct = false;
								break;
							}
						}
					}

					if (has_bool_construct && function_in.is_valid_label(block[i - 2]->instruction_.label_)) {
						for (std::uint32_t j = function_in.labels_[block[i - 2]->instruction_.label_].jump_ids_.size(); j--;) {
							target_index = this->get_block_index_from_id(block, function_in.labels_[block[i - 2]->instruction_.label_].jump_ids_[j] - 1);

							if (target_index == INVALID_ID || block[target_index]->type_ != statement::type::t_condition) {
								index = INVALID_ID;
								break;
							}

							if (!block[target_index]->assignment_.expressions_.size() || block[target_index]->assignment_.variables_.size()) {
								has_bool_construct = false;
								break;
							}
						}
					}

					if (index == INVALID_ID) {
						continue;
					}
				}
			}

			previous_valid_index = INVALID_ID;
			has_end_assignment = has_bool_construct ? block[i - 3]->type_ == statement::type::t_condition || block[i - 4]->type_ == statement::type::t_assignment : block[i]->type_ == statement::type::t_assignment;
			target_index = has_bool_construct ? (block[i - 3]->type_ == statement::type::t_goto ? i - (has_end_assignment ? 4 : 3) : i - 2) : (has_end_assignment ? i : i + 1);

			for (std::uint32_t j = function_in.labels_[target_label].jump_ids_.size(); j--;) {
				if (function_in.labels_[target_label].jump_ids_[j] > block[i]->instruction_.id_) {
					continue;
				}

				if (function_in.labels_[target_label].jump_ids_[j] < block[index]->instruction_.id_) {
					index = this->get_block_index_from_id(block, function_in.labels_[target_label].jump_ids_[j] - 1);

					if (has_bool_construct && index == i - 2 && !function_in.is_valid_label(block[i]->instruction_.label_)) {
						index = this->get_block_index_from_id(block, function_in.labels_[block[i - 2]->instruction_.label_].jump_ids_.front() - 1);
						if (index == INVALID_ID) {
							index = i - 2;
						}
					}
				}

				for (std::uint32_t k = i; index != INVALID_ID && block[index]->instruction_.id_ < block[k]->instruction_.id_; k--) {
					if (function_in.is_valid_label(block[k]->instruction_.label_)) {
						if (function_in.labels_[block[k]->instruction_.label_].jump_ids_.back() >= block[k]->instruction_.id_) {
							index = INVALID_ID;
							break;
						}

						while (function_in.labels_[block[k]->instruction_.label_].jump_ids_.front() < block[index]->instruction_.id_) {
							if (!index) {
								index = INVALID_ID;
								break;
							}

							index--;
						}
					}
				}

				if (index == INVALID_ID) {
					break;
				}

				switch (block[index]->type_) {
				case statement::type::t_goto:
				case statement::type::t_break:
					if (block[index]->instruction_.target_ == function_in.labels_[target_label].target_ && index) {
						index--;
					}
				default:
					break;
				}

				for (std::uint32_t k = index; k < target_index; k++) {
					switch (block[k]->type_) {
					case statement::type::t_condition:
						if (block[k]->assignment_.variables_.size()) {
							if (block[k]->instruction_.target_ == function_in.labels_[target_label].target_
								&& block[k]->assignment_.variables_.back().slot_ == block[assignment_index]->assignment_.variables_.back().slot_)
							{
								continue;
							}
						}
						else if (block[k]->instruction_.target_ == function_in.labels_[target_label].target_
							&& block[k]->assignment_.expressions_.size() == 1
							&& block[k]->assignment_.expressions_.back()->type_ == expression::type::t_variable
							&& block[k]->assignment_.expressions_.back()->variable_->type_ == variable::type::t_slot
							&& block[k]->assignment_.expressions_.back()->variable_->slot_ == block[assignment_index]->assignment_.variables_.back().slot_)
						{
							continue;
						}
						else if ((block[k]->instruction_.target_ == function_in.labels_[extended_target_label].target_ && !has_end_assignment)
							|| (block[k]->instruction_.target_ > block[k]->instruction_.id_ && block[k]->instruction_.target_ < function_in.labels_[target_label].target_))
						{
							continue;
						}

						break;
					case statement::type::t_assignment:
						if (block[k]->assignment_.variables_.size() == 1
							&& block[k]->assignment_.variables_.back().type_ == variable::type::t_slot
							&& block[k]->assignment_.variables_.back().slot_ == block[assignment_index]->assignment_.variables_.back().slot_
							&& block[k]->assignment_.expressions_.back()->type_ == expression::type::t_constant
							&& get_constant_type(block[k]->assignment_.expressions_.back()) != constant_type::t_invalid
							&& ++k != target_index
							&& (block[k]->type_ == statement::type::t_goto || block[k]->type_ == statement::type::t_break)
							&& !function_in.is_valid_label(block[k]->instruction_.label_)
							&& block[k]->instruction_.type_ == bytecode::opcode::op_jmp
							&& block[k]->instruction_.target_ == function_in.labels_[target_label].target_)
						{
							continue;
						}
						break;
					default:
						break;
					}

					index = INVALID_ID;
					break;
				}

				if (index == INVALID_ID) {
					break;
				}
				previous_valid_index = index;
			}

			if (previous_valid_index == INVALID_ID) {
				continue;
			}
			index = previous_valid_index;

			condition_builder conditionBuilder(condition_builder::type::t_assignment, *this, target_label,
				has_bool_construct ? block[i]->instruction_.label_ : INVALID_ID, has_bool_construct ? block[i - 2]->instruction_.label_ : INVALID_ID);

			for (std::uint32_t j = index; j < target_index; j++) {
				switch (block[j]->type_) {
				case statement::type::t_condition:
					conditionBuilder.add_node(conditionBuilder.get_node_type(block[j]->instruction_.type_, block[j]->condition_.swapped_), block[j]->instruction_.label_,
						has_end_assignment
						|| block[j]->assignment_.variables_.size()
						|| (block[j]->instruction_.target_ == function_in.labels_[target_label].target_
							? target_label != extended_target_label
							//TODO ???
							/*|| (block[j]->assignment.expressions.size() == 1
								&& block[j]->assignment.expressions.back()->type == AST_EXPRESSION_VARIABLE
								&& block[j]->assignment.expressions.back()->variable->type == AST_VARIABLE_SLOT
								&& block[j]->assignment.expressions.back()->variable->slot == block[assignmentIndex]->assignment.variables.back().slot)*/
							: block[j]->instruction_.target_ != function_in.labels_[extended_target_label].target_)
						? function_in.get_label_from_id(block[j]->instruction_.target_) : function_in.labels_.size(), &block[j]->assignment_.expressions_);
					continue;
				case statement::type::t_assignment:
					switch (block[j]->assignment_.expressions_.back()->constant_->type_) {
					case constant::type::t_nil:
					case constant::type::t_false:
						conditionBuilder.add_node(condition_builder::node::type::t_falsy_test, block[j]->instruction_.label_,
							function_in.get_label_from_id(block[j + 1]->instruction_.target_), &block[j]->assignment_.expressions_);
						break;
					case constant::type::t_true:
					case constant::type::t_string:
					case constant::type::t_number:
						conditionBuilder.add_node(condition_builder::node::type::t_truthy_test, block[j]->instruction_.label_,
							function_in.get_label_from_id(block[j + 1]->instruction_.target_), &block[j]->assignment_.expressions_);
						break;
					default:
						break;
					}
					j++;
					continue;
				default:
					break;
				}
			}

			if (has_end_assignment) {
				if (!has_bool_construct) {
					conditionBuilder.add_node(condition_builder::node::type::t_truthy_test, block[i]->instruction_.label_, target_label, &block[i]->assignment_.expressions_);
				}
				else if (block[i - 3]->type_ == statement::type::t_goto) {
					conditionBuilder.add_node(condition_builder::node::type::t_truthy_test, block[i - 4]->instruction_.label_, target_label, &block[i - 4]->assignment_.expressions_);
				}
			} else {
				expressions.back() = this->new_slot(block[assignment_index]->assignment_.variables_.back().slot_);
				expressions.back()->variable_->slot_scope_ = block[assignment_index]->assignment_.variables_.back().slot_scope_;
				conditionBuilder.add_node(condition_builder::node::type::t_truthy_test, function_in.labels_.size(), target_label, &expressions);
			}
			
			expressions.back() = conditionBuilder.build_condition();
			if (!expressions.back()) {
				continue;
			}
			block[assignment_index]->assignment_.expressions_.back() = expressions.back();

			for (std::uint32_t j = index; j <= i; j++) {
				switch (block[j]->type_) {
				case statement::type::t_condition:
					function_in.remove_jump(block[j]->instruction_.id_ + 1, block[j]->instruction_.target_);
					if (!block[j]->assignment_.variables_.size()) {
						continue;
					}
					function_in.remove_jump(block[j]->instruction_.id_, block[j]->instruction_.id_ + 2);
				case statement::type::t_assignment:
					if (*block[j]->assignment_.variables_.back().slot_scope_ != *block[assignment_index]->assignment_.variables_.back().slot_scope_) {
						(*block[assignment_index]->assignment_.variables_.back().slot_scope_)->usages_ += (*block[j]->assignment_.variables_.back().slot_scope_)->usages_;
						if ((*block[j]->assignment_.variables_.back().slot_scope_)->scope_begin_ < (*block[assignment_index]->assignment_.variables_.back().slot_scope_)->scope_begin_)
							(*block[assignment_index]->assignment_.variables_.back().slot_scope_)->scope_begin_ = (*block[j]->assignment_.variables_.back().slot_scope_)->scope_begin_;
						if ((*block[j]->assignment_.variables_.back().slot_scope_)->scope_end_ > (*block[assignment_index]->assignment_.variables_.back().slot_scope_)->scope_end_)
							(*block[assignment_index]->assignment_.variables_.back().slot_scope_)->scope_end_ = (*block[j]->assignment_.variables_.back().slot_scope_)->scope_end_;
						*block[j]->assignment_.variables_.back().slot_scope_ = *block[assignment_index]->assignment_.variables_.back().slot_scope_;
						if (block[j]->assignment_.variables_.back().slot_scope_ != block[assignment_index]->assignment_.variables_.back().slot_scope_)
							function_in.slot_scope_collector_.remove_scope(block[j]->assignment_.variables_.back().slot_, block[j]->assignment_.variables_.back().slot_scope_);
					}
					continue;
				case statement::type::t_goto:
				case statement::type::t_break:
					function_in.remove_jump(block[j]->instruction_.id_, block[j]->instruction_.target_);
					continue;
				default:
					break;
				}
			}
			block[i] = block[assignment_index];
			block[i]->type_ = statement::type::t_assignment;
			block[i]->instruction_.label_ = block[index]->instruction_.label_;
			if ((*block[i]->assignment_.variables_.back().slot_scope_)->scope_begin_ >= block[index]->instruction_.id_) {
				block[i]->assignment_.forward_declaration_ = true;
			}
			block.erase(block.begin() + index, block.begin() + i);
			i = index;
		}

		for (std::uint32_t i = block.size(); i--;) {
			switch (block[i]->type_) {
			case statement::type::t_condition:
				blockInfo.index_ = i;
				target_label = this->get_label_from_next_statement(function_in, blockInfo, true, false);
				target_index = INVALID_ID;
				index = i;

				while (index && block[index - 1]->type_ == statement::type::t_condition) {
					index--;
				}

				for (std::uint32_t j = index; j <= i; j++) {
					if (function_in.is_valid_label(block[j]->instruction_.label_)) {
						if (function_in.labels_[block[j]->instruction_.label_].jump_ids_.front() < block[index]->instruction_.id_
							|| function_in.labels_[block[j]->instruction_.label_].jump_ids_.back() > block[j]->instruction_.id_)
						{
							index = j;
							target_index = INVALID_ID;
						}
						else if ((j && j - 1 >= index
							&& block[j - 1]->instruction_.target_ == function_in.labels_[block[j]->instruction_.label_].target_))
						{
							for (std::uint32_t k = index; k < j
								&& block[k]->instruction_.target_ > block[k]->instruction_.id_
								&& block[k]->instruction_.target_ <= block[j]->instruction_.id_; k++) {
								if (k != j - 1) continue;
								index = j;
								target_index = INVALID_ID;
								break;
							}
						}
					}

					if ((target_label == INVALID_ID
						|| block[j]->instruction_.target_ != function_in.labels_[target_label].target_)
						&& (block[j]->instruction_.target_ < block[j]->instruction_.id_
							|| block[j]->instruction_.target_ > block[i]->instruction_.id_))
					{
						if (target_index != INVALID_ID) {
							if (block[j]->instruction_.target_ == block[target_index]->instruction_.target_) {
								continue;
							}
							index = target_index + 1;
							j = target_index;
							target_index = INVALID_ID;
							continue;
						}

						target_index = j;
					}
				}

				if (target_index == INVALID_ID) {
					extended_target_label = target_label;
					target_label = INVALID_ID;
				}
				else {
					extended_target_label = function_in.get_label_from_id(block[target_index]->instruction_.target_);
				}

				{
					condition_builder builder(condition_builder::type::t_statement, *this, INVALID_ID, target_label, extended_target_label);

					for (std::uint32_t j = index; j <= i; j++) {
						// ld_assert(!block[j]->assignment_.variables_.size(), "Failed to eliminate all test and copy conditions", this->bc_.file_path_);
						if (!block[j]->assignment_.variables_.empty()) {
							LOG("LuaDecompiler", WARN, "[{}]: Failed to eliminate all test and copy conditions (on block {})", this->bc_.file_path_, j);
						}
						builder.add_node(builder.get_node_type(block[j]->instruction_.type_, block[j]->condition_.swapped_),
							block[j]->instruction_.label_, function_in.get_label_from_id(block[j]->instruction_.target_), &block[j]->assignment_.expressions_);
					}

					expressions.back() = builder.build_condition();
					ld_assert(expressions.back(), "Failed to build condition", this->bc_.file_path_);
					block[i]->assignment_.expressions_ = expressions;

					for (std::uint32_t j = index; j <= i; j++) {
						function_in.remove_jump(block[j]->instruction_.id_ + 1, block[j]->instruction_.target_);
					}

					block[i]->instruction_.target_ = function_in.labels_[extended_target_label].target_;
					function_in.add_jump(block[i]->instruction_.id_, block[i]->instruction_.target_);
					block[i]->instruction_.label_ = block[index]->instruction_.label_;
					block.erase(block.begin() + index, block.begin() + i);
					i = index;
				}

				if (i
					&& block[i]->instruction_.type_ == bytecode::opcode::op_jmp
					&& block[i]->assignment_.expressions_.back()->type_ == expression::type::t_constant
					&& block[i]->assignment_.expressions_.back()->constant_->type_ == constant::type::t_false
					&& !function_in.is_valid_label(block[i]->instruction_.label_)
					&& block[i - 1]->type_ == statement::type::t_assignment
					&& block[i - 1]->assignment_.variables_.size() == 1
					&& block[i - 1]->assignment_.variables_.back().type_ == variable::type::t_slot
					&& block[i - 1]->assignment_.expressions_.size() == 1
					&& get_constant_type(block[i - 1]->assignment_.expressions_.back()) != constant_type::t_invalid)
				{
					//TODO
					function_in.remove_jump(block[i]->instruction_.id_, block[i]->instruction_.target_);
					block[i]->assignment_.expressions_.clear();
					block[i]->type_ = statement::type::t_goto;
					block.emplace(block.begin() + i, this->new_statement(ENUM_UNDER(statement::type::t_goto)));
					block[i]->instruction_.type_ = bytecode::opcode::op_jmp;
					block[i]->instruction_.id_ = block[i + 1]->instruction_.id_;
					block[i + 1]->instruction_.id_++;
					block[i]->instruction_.target_ = block[i + 1]->instruction_.id_;
					function_in.add_jump(block[i]->instruction_.id_, block[i]->instruction_.target_);
					function_in.add_jump(block[i + 1]->instruction_.id_, block[i + 1]->instruction_.target_);
					block[i + 1]->instruction_.label_ = function_in.get_label_from_id(block[i + 1]->instruction_.id_);
				}
				continue;
			case statement::type::t_numeric_for:
			case statement::type::t_generic_for:
				this->eliminate_conditions(function_in, block[i]->block_, nullptr);
				continue;
			case statement::type::t_loop:
			case statement::type::t_declaration:
				blockInfo.index_ = i;
				this->eliminate_conditions(function_in, block[i]->block_, &blockInfo);
				continue;
			default:
				break;
			}
		}
		return this->build_multi_assignment(function_in, block);
	}

	void ast::build_multi_assignment(function& function_in, std::vector<statement*>& block) {
		bool isMultiAssignment;
		std::uint32_t index;

		for (std::uint32_t i = block.size(); i--;) {
			switch (block[i]->type_) {
			case statement::type::t_assignment:
				if (block[i]->assignment_.variables_.size() >= 2) {
					if (i + block[i]->assignment_.variables_.size() >= block.size()) {
						continue;
					}
					isMultiAssignment = true;
					for (std::uint8_t j = block[i]->assignment_.variables_.size(); j--;) {
						if ((*block[i]->assignment_.variables_[j].slot_scope_)->usages_ == 1
							&& !function_in.is_valid_label(block[i + block[i]->assignment_.variables_.size() - j]->instruction_.label_)
							&& block[i + block[i]->assignment_.variables_.size() - j]->type_ == statement::type::t_assignment
							&& block[i + block[i]->assignment_.variables_.size() - j]->assignment_.variables_.size() == 1
							&& (block[i + block[i]->assignment_.variables_.size() - j]->assignment_.variables_.back().type_ != variable::type::t_table_index
								|| (block[i + block[i]->assignment_.variables_.size() - j]->assignment_.variables_.back().table_->type_ == expression::type::t_variable
									&& block[i + block[i]->assignment_.variables_.size() - j]->assignment_.variables_.back().table_->variable_->type_ == variable::type::t_slot
									&& (get_constant_type(block[i + block[i]->assignment_.variables_.size() - j]->assignment_.variables_.back().table_index_) != constant_type::t_invalid
										|| (block[i + block[i]->assignment_.variables_.size() - j]->assignment_.variables_.back().table_index_->type_ == expression::type::t_variable
											&& block[i + block[i]->assignment_.variables_.size() - j]->assignment_.variables_.back().table_index_->variable_->type_ == variable::type::t_slot))))
							&& block[i + block[i]->assignment_.variables_.size() - j]->assignment_.expressions_.size() == 1
							&& block[i + block[i]->assignment_.variables_.size() - j]->assignment_.expressions_.back()->type_ == expression::type::t_variable
							&& block[i + block[i]->assignment_.variables_.size() - j]->assignment_.expressions_.back()->variable_->type_ == variable::type::t_slot
							&& block[i + block[i]->assignment_.variables_.size() - j]->assignment_.expressions_.back()->variable_->slot_scope_ == block[i]->assignment_.variables_[j].slot_scope_)
						{
							continue;
						}
						isMultiAssignment = false;
						break;
					}
					if (!isMultiAssignment) {
						continue;
					}
					for (std::uint8_t j = block[i]->assignment_.variables_.size(); j--;) {
						function_in.slot_scope_collector_.remove_scope(block[i]->assignment_.variables_[j].slot_, block[i]->assignment_.variables_[j].slot_scope_);
						block[i]->assignment_.variables_[j] = block[i + 1]->assignment_.variables_.back();
						block.erase(block.begin() + i + 1);
					}
					break;
				}
			case statement::type::t_function_call:
				index = i;
				if (block[i]->type_ == statement::type::t_function_call
					|| (block[i]->assignment_.variables_.back().type_ == variable::type::t_slot
						&& !(*block[i]->assignment_.variables_.back().slot_scope_)->usages_
						&& !block[i]->assignment_.forward_declaration_))
				{
					while (index
						&& !function_in.is_valid_label(block[index]->instruction_.label_)
						&& block[index - 1]->type_ == statement::type::t_assignment
						&& block[index - 1]->assignment_.variables_.size() == 1
						&& block[index - 1]->assignment_.variables_.back().type_ == variable::type::t_slot
						&& !(*block[index - 1]->assignment_.variables_.back().slot_scope_)->usages_
						&& !block[index - 1]->assignment_.forward_declaration_)
					{
						index--;
					}
				}
				if (index
					&& i + 1 < block.size()
					&& !function_in.is_valid_label(block[index]->instruction_.label_)
					&& !function_in.is_valid_label(block[i + 1]->instruction_.label_)
					&& block[index - 1]->type_ == statement::type::t_assignment
					&& block[index - 1]->assignment_.variables_.size() == 1
					&& block[index - 1]->assignment_.variables_.back().type_ == variable::type::t_slot
					&& (*block[index - 1]->assignment_.variables_.back().slot_scope_)->usages_ == 1
					&& block[i + 1]->type_ == statement::type::t_assignment
					&& block[i + 1]->assignment_.variables_.size() == 1
					&& (block[i + 1]->assignment_.variables_.back().type_ != variable::type::t_table_index
						|| (block[i + 1]->assignment_.variables_.back().table_->type_ == expression::type::t_variable
							&& block[i + 1]->assignment_.variables_.back().table_->variable_->type_ == variable::type::t_slot
							&& (get_constant_type(block[i + 1]->assignment_.variables_.back().table_index_) != constant_type::t_invalid
								|| (block[i + 1]->assignment_.variables_.back().table_index_->type_ == expression::type::t_variable
									&& block[i + 1]->assignment_.variables_.back().table_index_->variable_->type_ == variable::type::t_slot))))
					&& block[i + 1]->assignment_.expressions_.size() == 1
					&& block[i + 1]->assignment_.expressions_.back()->type_ == expression::type::t_variable
					&& block[i + 1]->assignment_.expressions_.back()->variable_->type_ == variable::type::t_slot
					&& block[i + 1]->assignment_.expressions_.back()->variable_->slot_scope_ == block[index - 1]->assignment_.variables_.back().slot_scope_)
				{
					if (block[i]->type_ == statement::type::t_assignment) {
						switch (block[i]->assignment_.variables_.back().type_) {
						case variable::type::t_slot:
							if (!(*block[i]->assignment_.variables_.back().slot_scope_)->usages_ && !block[i]->assignment_.forward_declaration_) {
								function_in.slot_scope_collector_.remove_scope(block[i]->assignment_.variables_.back().slot_, block[i]->assignment_.variables_.back().slot_scope_);
								block[i]->assignment_.variables_.clear();
							}
							break;
						case variable::type::t_table_index:
							if (index == i
								&& (block[i]->assignment_.variables_.back().table_->type_ != expression::type::t_variable
									|| block[i]->assignment_.variables_.back().table_->variable_->type_ != variable::type::t_slot
									|| (get_constant_type(block[i]->assignment_.variables_.back().table_index_) == constant_type::t_invalid
										&& (block[i]->assignment_.variables_.back().table_index_->type_ != expression::type::t_variable
											|| block[i]->assignment_.variables_.back().table_index_->variable_->type_ != variable::type::t_slot))))
							{
								continue;
							}
							break;
						default:
							break;
						}
					}
					else {
						block[i]->type_ = statement::type::t_assignment;
					}
					while (i != index) {
						i--;
						function_in.slot_scope_collector_.remove_scope(block[i]->assignment_.variables_.back().slot_, block[i]->assignment_.variables_.back().slot_scope_);
						block[i + 1]->assignment_.expressions_.emplace(block[i + 1]->assignment_.expressions_.begin(), block[i]->assignment_.expressions_.back());
						block.erase(block.begin() + i);
					}
					break;
				}
				if (block[i]->type_ == statement::type::t_function_call && block[i]->assignment_.expressions_.back()->type_ == expression::type::t_vararg) {
					ld_assert(i
						&& !function_in.is_valid_label(block[i]->instruction_.label_)
						&& block[i - 1]->type_ == statement::type::t_assignment
						&& block[i - 1]->assignment_.variables_.size() == 1,
						"Unable to eliminate vararg with zero returns", this->bc_.file_path_);
					block[i - 1]->assignment_.expressions_.emplace_back(block[i]->assignment_.expressions_.back());
					block.erase(block.begin() + i);
					i--;
				}
			default:
				continue;
			}

			while (i
				&& i + 1 < block.size()
				&& !function_in.is_valid_label(block[i]->instruction_.label_)
				&& !function_in.is_valid_label(block[i + 1]->instruction_.label_)
				&& block[i - 1]->type_ == statement::type::t_assignment
				&& block[i - 1]->assignment_.variables_.size() == 1
				&& block[i - 1]->assignment_.variables_.back().type_ == variable::type::t_slot
				&& (*block[i - 1]->assignment_.variables_.back().slot_scope_)->usages_ == 1
				&& block[i + 1]->type_ == statement::type::t_assignment
				&& block[i + 1]->assignment_.variables_.size() == 1
				&& (block[i + 1]->assignment_.variables_.back().type_ != variable::type::t_table_index
					|| (block[i + 1]->assignment_.variables_.back().table_->type_ == expression::type::t_variable
						&& block[i + 1]->assignment_.variables_.back().table_->variable_->type_ == variable::type::t_slot
						&& (get_constant_type(block[i + 1]->assignment_.variables_.back().table_index_) != constant_type::t_invalid
							|| (block[i + 1]->assignment_.variables_.back().table_index_->type_ == expression::type::t_variable
								&& block[i + 1]->assignment_.variables_.back().table_index_->variable_->type_ == variable::type::t_slot))))
				&& block[i + 1]->assignment_.expressions_.size() == 1
				&& block[i + 1]->assignment_.expressions_.back()->type_ == expression::type::t_variable
				&& block[i + 1]->assignment_.expressions_.back()->variable_->type_ == variable::type::t_slot
				&& block[i + 1]->assignment_.expressions_.back()->variable_->slot_scope_ == block[i - 1]->assignment_.variables_.back().slot_scope_)
			{
				function_in.slot_scope_collector_.remove_scope(block[i - 1]->assignment_.variables_.back().slot_, block[i - 1]->assignment_.variables_.back().slot_scope_);
				block[i]->assignment_.expressions_.emplace(block[i]->assignment_.expressions_.begin(), block[i - 1]->assignment_.expressions_.back());
				block[i]->assignment_.variables_.emplace(block[i]->assignment_.variables_.begin(), block[i + 1]->assignment_.variables_.back());
				block[i]->instruction_.label_ = block[i - 1]->instruction_.label_;
				block.erase(block.begin() + i - 1);
				block.erase(block.begin() + i);
				i--;
			}

			for (std::uint8_t j = block[i]->assignment_.variables_.size(); j--
				&& i
				&& !function_in.is_valid_label(block[i]->instruction_.label_)
				&& block[i - 1]->type_ == statement::type::t_assignment
				&& block[i - 1]->assignment_.variables_.size() == 1
				&& block[i - 1]->assignment_.variables_.back().type_ == variable::type::t_slot
				&& (*block[i - 1]->assignment_.variables_.back().slot_scope_)->usages_ == 1;)
			{
				if (block[i]->assignment_.variables_[j].type_ != variable::type::t_table_index) {
					continue;
				}
				if (block[i]->assignment_.variables_[j].table_index_->type_ == expression::type::t_variable
					&& block[i]->assignment_.variables_[j].table_index_->variable_->type_ == variable::type::t_slot
					&& block[i]->assignment_.variables_[j].table_index_->variable_->slot_scope_ == block[i - 1]->assignment_.variables_.back().slot_scope_)
				{
					function_in.slot_scope_collector_.remove_scope(block[i - 1]->assignment_.variables_.back().slot_, block[i - 1]->assignment_.variables_.back().slot_scope_);
					block[i]->assignment_.variables_[j].table_index_ = block[i - 1]->assignment_.expressions_.back();
					block[i]->instruction_.label_ = block[i - 1]->instruction_.label_;
					i--;
					block.erase(block.begin() + i);
					j++;
					continue;
				}
				if (block[i]->assignment_.variables_[j].table_->type_ == expression::type::t_variable && block[i]->assignment_.variables_[j].table_->variable_->slot_scope_ == block[i - 1]->assignment_.variables_.back().slot_scope_) {
					function_in.slot_scope_collector_.remove_scope(block[i - 1]->assignment_.variables_.back().slot_, block[i - 1]->assignment_.variables_.back().slot_scope_);
					block[i]->assignment_.variables_[j].table_ = block[i - 1]->assignment_.expressions_.back();
					block[i]->instruction_.label_ = block[i - 1]->instruction_.label_;
					i--;
					block.erase(block.begin() + i);
				}
			}
		}
	}

	void ast::build_if_statements_from_map(function& function_in, std::vector<statement*>& block, block_info* const& previous_block,
		std::unordered_map<statement*, std::uint32_t>& offset_map)
	{
		block_info info = {
			.block_ = block,
			.previous_block_ = previous_block
		};
		std::uint32_t index;
		for (auto x = 0; x < block.size(); x++) {
			switch (block[x]->type_) {
			case statement::type::t_goto:
				if (!offset_map.contains(block[x])) {
					continue;
				}
			case statement::type::t_condition:
				function_in.remove_jump(block[x]->instruction_.id_, block[x]->instruction_.target_);
				index = offset_map[block[x]] + x;

				if (block[x]->type_ == statement::type::t_goto && block[x]->instruction_.type_ == bytecode::opcode::op_jmp) {
					block[x]->type_ = statement::type::t_empty;
					x++;
					index++;
					block.emplace(block.begin() + x, this->new_statement(ENUM_UNDER(statement::type::t_goto)));
					block[x]->instruction_.id_ = block[x - 1]->instruction_.id_;
					block[x - 1]->instruction_.id_ = INVALID_ID;
				}

				block[x]->block_.reserve(index - x);
				block[x]->block_.insert(block[x]->block_.begin(), block.begin() + x + 1, block.begin() + index + 1);
				block.erase(block.begin() + x + 1, block.begin() + index + 1);

				if (block[x]->type_ == statement::type::t_condition
					&& block[x]->block_.size()
					&& block[x]->block_.back()->type_ == statement::type::t_goto
					&& block[x]->block_.back()->instruction_.type_ != bytecode::opcode::op_loop)
				{
					index = offset_map[block[x]->block_.back()] + x;
					block.emplace(block.begin() + x + 1, this->new_statement(ENUM_UNDER(statement::type::t_else)));
					block[x + 1]->block_.reserve(index - x);
					block[x + 1]->block_.insert(block[x + 1]->block_.begin(), block.begin() + x + 2, block.begin() + index + 2);
					block.erase(block.begin() + x + 2, block.begin() + index + 2);
					function_in.remove_jump(block[x]->block_.back()->instruction_.id_, block[x]->block_.back()->instruction_.target_);
					block[x]->block_.back()->type_ = statement::type::t_empty;
					info.index_ = x + 1;
					this->build_if_statements_from_map(function_in, block[x + 1]->block_, &info, offset_map);
				}

				if (block[x]->type_ == statement::type::t_goto) {
					block[x]->assignment_.expressions_.emplace_back(this->new_primitive(1));
				}
				block[x]->type_ = statement::type::t_if;
				info.index_ = x;
				this->build_if_statements_from_map(function_in, block[x]->block_, &info, offset_map);
				continue;
			case statement::type::t_numeric_for:
			case statement::type::t_generic_for:
				this->build_if_statements(function_in, block[x]->block_, nullptr);
				continue;
			case statement::type::t_loop:
			case statement::type::t_declaration:
				info.index_ = x;
				this->build_if_statements(function_in, block[x]->block_, &info);
			default:
				continue;
			}
		}
	}

	void ast::build_if_statements(function& function_in, std::vector<statement*>& block, block_info* const& previous_block) {
		const auto build_if_false_statements = [&](std::vector<statement*>& block, block_info* const& previous_block) -> void {
			block_info info = {
				.block_ = block,
				.previous_block_ = previous_block
			};
			std::uint32_t index, target_label;
			for (auto x = block.size(); x--;) {
				if (block[x]->type_ != statement::type::t_goto || block[x]->instruction_.type_ == bytecode::opcode::op_loop) {
					continue;
				}
				target_label = INVALID_ID;
				for (index = x; index < block.size(); index++) {
					info.index_ = index;
					target_label = this->get_label_from_next_statement(function_in, info, false, false);
					if (target_label == INVALID_ID || function_in.labels_[target_label].target_ != block[x]->instruction_.target_) {
						target_label = this->get_label_from_next_statement(function_in, info, true, false);
					}
					if (target_label == INVALID_ID) {
						continue;
					}
					if (function_in.labels_[target_label].target_ == block[x]->instruction_.target_
						&& this->is_valid_block(function_in, info, block[x]->instruction_.id_ + 1))
					{
						break;
					}
					target_label = INVALID_ID;
				}

				if (target_label == INVALID_ID) {
					continue;
				}
				function_in.remove_jump(block[x]->instruction_.id_, block[x]->instruction_.target_);

				if (block[x]->instruction_.type_ == bytecode::opcode::op_jmp) {
					block[x]->type_ = statement::type::t_empty;
					x++;
					index++;
					block.emplace(block.begin() + x, this->new_statement(ENUM_UNDER(statement::type::t_if)));
					block[x]->instruction_.id_ = block[x - 1]->instruction_.id_;
					block[x - 1]->instruction_.id_ = INVALID_ID;
				}
				else {
					block[x]->type_ = statement::type::t_if;
				}

				block[x]->assignment_.expressions_.emplace_back(this->new_primitive(1));
				block[x]->block_.reserve(index - x);
				block[x]->block_.insert(block[x]->block_.begin(), block.begin() + x + 1, block.begin() + index + 1);
				block.erase(block.begin() + x + 1, block.begin() + index + 1);
			}
		};

		const auto build_else_statements = [&](std::vector<statement*>& block, block_info* const& previous_block) -> void {
			block_info info = {
				.block_ = block,
				.previous_block_ = previous_block
			};
			std::uint32_t index, target_label;
			for (auto i = block.size(); i--;) {
				if (block[i]->type_ != statement::type::t_if) {
					continue;
				}
				if (block[i]->block_.size() && block[i]->block_.back()->type_ == statement::type::t_goto
					&& block[i]->block_.back()->instruction_.type_ != bytecode::opcode::op_loop)
				{
					target_label = INVALID_ID;
					for (index = i; index < block.size(); index++) {
						info.index_ = index;
						target_label = this->get_label_from_next_statement(function_in, info, false, false);
						if (target_label == INVALID_ID || function_in.labels_[target_label].target_ != block[i]->block_.back()->instruction_.target_) {
							target_label = get_label_from_next_statement(function_in, info, true, false);
						}
						if (target_label == INVALID_ID) {
							continue;
						}
						if (function_in.labels_[target_label].target_ == block[i]->block_.back()->instruction_.target_
							&& this->is_valid_block(function_in, info, block[i]->block_.back()->instruction_.id_ + 1))
						{
							break;
						}
						target_label = INVALID_ID;
					}

					if (target_label != INVALID_ID) {
						block.emplace(block.begin() + i + 1, this->new_statement(ENUM_UNDER(statement::type::t_else)));
						block[i + 1]->block_.reserve(index - i);
						block[i + 1]->block_.insert(block[i + 1]->block_.begin(), block.begin() + i + 2, block.begin() + index + 2);
						block.erase(block.begin() + i + 2, block.begin() + index + 2);
						function_in.remove_jump(block[i]->block_.back()->instruction_.id_, block[i]->block_.back()->instruction_.target_);
						block[i]->block_.back()->type_ = statement::type::t_empty;
						info.index_ = i + 1;
						build_if_false_statements(block[i + 1]->block_, &info);
						build_if_false_statements(block[i]->block_, nullptr);
						continue;
					}
				}
				info.index_ = i;
				build_if_false_statements(block[i]->block_, &info);
			}
		};

		block_info info = {
			.block_ = block,
			.previous_block_ = previous_block
		};
		std::uint32_t index, target_label;
		std::vector<std::uint32_t> indexes{};
		std::unordered_map<statement*, std::uint32_t> offset_map{};
		for (auto i = 0; i < block.size(); i++) {
			if (indexes.size() && (i == indexes.back() || block[i]->type_ != statement::type::t_condition)) {
				info.index_ = i;
				target_label = this->get_label_from_next_statement(function_in, info, false, false);
				if (target_label == INVALID_ID || function_in.labels_[target_label].target_ != block[indexes.back()]->instruction_.target_) {
					target_label = this->get_label_from_next_statement(function_in, info, true, false);
				}
				if (target_label != INVALID_ID && function_in.labels_[target_label].target_ == block[indexes.back()]->instruction_.target_
					&& this->is_valid_block(function_in, info, block[indexes.back()]->instruction_.id_
						+ (block[indexes.back()]->type_ == statement::type::t_condition ? 2 : 1)))
				{
					offset_map.emplace(block[indexes.back()], i - indexes.back());
					if (i - indexes.back() && block[indexes.back()]->type_ == statement::type::t_condition && block[i]->type_ == statement::type::t_goto
						&& block[i]->instruction_.type_ != bytecode::opcode::op_loop)
					{
						function_in.remove_jump(block[indexes.back()]->instruction_.id_, block[indexes.back()]->instruction_.target_);
						indexes.emplace_back(i);
						i--;
						continue;
					}
					if (indexes.size() >= 2 && offset_map.contains(block[indexes[indexes.size() - 2]])) {
						indexes.pop_back();
						function_in.add_jump(block[indexes.back()]->instruction_.id_, block[indexes.back()]->instruction_.target_);
					}
					indexes.pop_back();
					i--;
					continue;
				}
				if (i == indexes.back()) {
					continue;
				}
			}
			switch (block[i]->type_) {
			case statement::type::t_goto:
				if (block[i]->instruction_.type_ == bytecode::opcode::op_loop) {
					continue;
				}
			case statement::type::t_condition:
				if (offset_map.contains(block[i])) {
					continue;
				}
				indexes.emplace_back(i);
				i--;
			default:
				break;
			}
		}
		if (indexes.size() == 1 && block[indexes.back()]->type_ == statement::type::t_goto && indexes.back() == block.size() - 1 && previous_block
			&& previous_block->block_[previous_block->index_]->type_ == statement::type::t_loop)
		{
			indexes.pop_back();
		}
		if (!indexes.size()) {
			return this->build_if_statements_from_map(function_in, block, previous_block, offset_map);
		}
		for (auto i = indexes.size(); i--;) {
			if (offset_map.contains(block[indexes[i]])) {
				function_in.add_jump(block[indexes[i]]->instruction_.id_, block[indexes[i]]->instruction_.target_);
			}
		}

		for (auto i = block.size(); i--;) {
			switch (block[i]->type_) {
			case statement::type::t_condition:
				block[i]->type_ = statement::type::t_if;
				target_label = INVALID_ID;
				for (index = i; index < block.size(); index++) {
					info.index_ = index;
					target_label = this->get_label_from_next_statement(function_in, info, false, false);
					if (target_label == INVALID_ID || function_in.labels_[target_label].target_ != block[i]->instruction_.target_) {
						target_label = this->get_label_from_next_statement(function_in, info, true, false);
					}
					if (target_label == INVALID_ID) {
						continue;
					}
					if (function_in.labels_[target_label].target_ == block[i]->instruction_.target_
						&& this->is_valid_block(function_in, info, block[i]->instruction_.id_ + 2))
					{
						break;
					}
					target_label = INVALID_ID;
				}
				// ld_assert(target_label != INVALID_ID, "Failed to build if statement", this->bc_.file_path_);
				if (target_label == INVALID_ID) {
					LOG("LuaDecompiler", WARN, "[{}]: Failed to build 'if' statement (on block {})", this->bc_.file_path_, i);
					continue;
				}
				block[i]->block_.reserve(index - i);
				block[i]->block_.insert(block[i]->block_.begin(), block.begin() + i + 1, block.begin() + index + 1);
				block.erase(block.begin() + i + 1, block.begin() + index + 1);
				function_in.remove_jump(block[i]->instruction_.id_, block[i]->instruction_.target_);
				info.index_ = i;
				build_else_statements(block[i]->block_, &info);
				continue;
			case statement::type::t_numeric_for:
			case statement::type::t_generic_for:
				this->build_if_statements(function_in, block[i]->block_, nullptr);
				continue;
			case statement::type::t_loop:
			case statement::type::t_declaration:
				info.index_ = i;
				this->build_if_statements(function_in, block[i]->block_, &info);
				continue;
			default:
				break;
			}
		}

		build_else_statements(block, previous_block);
		build_if_false_statements(block, previous_block);
	}

	void ast::clean_up(function& function_in) {
		if (function_in.has_debug_info_) {
			for (auto x = function_in.parameter_names_.size(); x--;) {
				(*function_in.slot_scope_collector_.slot_infos_[x].active_slot_scope_)->name_ = function_in.parameter_names_[x];
			}
		}
		else {
			function_in.parameter_names_.resize(function_in.prototype_.header_.parameters_);
			for (auto x = function_in.parameter_names_.size(); x--;) {
				function_in.parameter_names_[x] = "arg_" + std::to_string(this->minimize_diffs_ ? function_in.level_ : function_in.id_)
					+ "_" + std::to_string(x);
				(*function_in.slot_scope_collector_.slot_infos_[x].active_slot_scope_)->name_ = function_in.parameter_names_[x];
			}
		}

		std::uint32_t variable_counter = 0, iterator_counter = 0;
		this->clean_up_block(function_in, function_in.block_, variable_counter, iterator_counter, nullptr);
		for (auto x = 0, label_counter = 0; x < function_in.labels_.size(); x++) {
			if (!function_in.labels_[x].jump_ids_.size()) {
				continue;
			}
			function_in.labels_[x].name_ = "label_" + std::to_string(this->minimize_diffs_ ? function_in.level_ : function_in.id_) + "_"
				+ std::to_string(label_counter);
			label_counter++;
		}
	}

	void ast::clean_up_block(function& function_in, std::vector<statement*>& block, std::uint32_t& variable_counter, std::uint32_t& iterator_counter,
		block_info* const& previous_block)
	{
		// TODO
		block_info blockInfo = {
			.block_ = block,
			.previous_block_ = previous_block
		};
		std::vector<variable*> declarations;
		statement** declarationTarget;

		for (auto i = 0; i < block.size(); i++) {
			switch (block[i]->type_) {
			case statement::type::t_numeric_for:
				if (block[i]->assignment_.expressions_.back()->type_ == expression::type::t_constant
					&& block[i]->assignment_.expressions_.back()->constant_->type_ == constant::type::t_number
					&& block[i]->assignment_.expressions_.back()->constant_->number_ == 1)
				{
					block[i]->assignment_.expressions_.pop_back();
				}
			case statement::type::t_generic_for:
				if (block[i]->type_ == statement::type::t_generic_for) {
					while (block[i]->assignment_.expressions_.size() > 1
						&& block[i]->assignment_.expressions_.back()->type_ == expression::type::t_constant
						&& block[i]->assignment_.expressions_.back()->constant_->type_ == constant::type::t_nil)
					{
						block[i]->assignment_.expressions_.pop_back();
					}
				}
				if (function_in.has_debug_info_) {
					for (auto j = block[i]->assignment_.variables_.size(); j--;) {
						(*block[i]->assignment_.variables_[j].slot_scope_)->name_ = block[i]->locals_->names_[j];
					}
				}
				else {
					for (auto j = 0; j < block[i]->assignment_.variables_.size(); j++) {
						(*block[i]->assignment_.variables_[j].slot_scope_)->name_ = "iter_"
							+ std::to_string(this->minimize_diffs_ ? function_in.level_ : function_in.id_) + "_"
							+ std::to_string(iterator_counter);
						iterator_counter++;
					}
				}
				this->clean_up_block(function_in, block[i]->block_, variable_counter, iterator_counter, nullptr);
				continue;
			case statement::type::t_loop:
				for (auto* current_block = &block[i]->block_; current_block->size(); current_block = &current_block->back()->block_) {
					if (current_block->back()->type_ == statement::type::t_declaration) {
						continue;
					}
					if (current_block->back()->type_ != statement::type::t_goto
						|| current_block->back()->instruction_.target_ != block[i]->instruction_.id_
						|| function_in.is_valid_label(current_block->back()->instruction_.label_)
						|| current_block->size() < 2)
					{
						break;
					}
					switch ((*current_block)[current_block->size() - 2]->type_) {
					case statement::type::t_break:
						function_in.remove_jump(current_block->back()->instruction_.id_, current_block->back()->instruction_.target_);
						function_in.remove_jump((*current_block)[current_block->size() - 2]->instruction_.id_,
							(*current_block)[current_block->size() - 2]->instruction_.target_);
						block[i]->type_ = statement::type::t_repeat;
						block[i]->assignment_.expressions_.emplace_back(new_primitive(2));
						(*current_block)[current_block->size() - 2]->type_ = statement::type::t_empty;
						current_block->erase(current_block->begin() + current_block->size() - 1);
						break;
					case statement::type::t_if:
						if ((*current_block)[current_block->size() - 2]->block_.size() != 1
							|| (*current_block)[current_block->size() - 2]->block_.back()->type_ != statement::type::t_break)
						{
							break;
						}
						function_in.remove_jump(current_block->back()->instruction_.id_, current_block->back()->instruction_.target_);
						function_in.remove_jump((*current_block)[current_block->size() - 2]->block_.back()->instruction_.id_,
							(*current_block)[current_block->size() - 2]->block_.back()->instruction_.target_);
						block[i]->type_ = statement::type::t_repeat;
						block[i]->assignment_.expressions_.emplace_back((*current_block)[current_block->size() - 2]->assignment_.expressions_.back());
						(*current_block)[current_block->size() - 2]->type_ = statement::type::t_empty;
						current_block->erase(current_block->begin() + current_block->size() - 1);
						break;
					default:
						break;
					}
					break;
				}
				if (block[i]->type_ == statement::type::t_loop) {
					if (block[i]->block_.size() && block[i]->block_.back()->type_ == statement::type::t_goto) {
						if (block[i]->instruction_.id_ == block[i]->block_.back()->instruction_.target_) {
							function_in.remove_jump(block[i]->block_.back()->instruction_.id_, block[i]->block_.back()->instruction_.target_);
							block[i]->type_ = statement::type::t_while;
							block[i]->assignment_.expressions_.emplace_back(this->new_primitive(2));
							block[i]->block_.back()->type_ = statement::type::t_empty;
						}
						else if (block.size() != 1) {
							for (auto j = i; j-- && block[j]->type_ == statement::type::t_if && !block[j]->block_.size();) {
								if (!function_in.is_valid_label(block[j]->instruction_.label_)) {
									continue;
								}
								if (function_in.is_valid_label(block[i]->instruction_.label_)
									|| function_in.labels_[block[j]->instruction_.label_].target_ != block[i]->block_.back()->instruction_.target_)
								{
									break;
								}
								function_in.remove_jump(block[i]->block_.back()->instruction_.id_, block[i]->block_.back()->instruction_.target_);
								block[i]->type_ = statement::type::t_while;
								block[i]->instruction_.label_ = block[j]->instruction_.label_;

								for (expression* expr; i != j; i--) {
									expr = this->new_expression(ENUM_UNDER(expression::type::t_binary_operation));
									expr->binary_operation_->type_ = binary_operation::type::t_or;
									expr->binary_operation_->right_operand_ = this->new_primitive(2);
									expr->binary_operation_->left_operand_ = block[i - 1]->assignment_.expressions_.back();

									if (block[i]->assignment_.expressions_.size()) {
										block[i - 1]->assignment_.expressions_.back()
											= this->new_expression(ENUM_UNDER(expression::type::t_binary_operation));
										block[i - 1]->assignment_.expressions_.back()->binary_operation_->type_
											= binary_operation::type::t_and;
										block[i - 1]->assignment_.expressions_.back()->binary_operation_->right_operand_
											= block[i]->assignment_.expressions_.back();
										block[i - 1]->assignment_.expressions_.back()->binary_operation_->left_operand_ = expr;
										block[i]->assignment_.expressions_.back() = block[i - 1]->assignment_.expressions_.back();
									}
									else {
										block[i]->assignment_.expressions_.resize(1, expr);
									}

									block.erase(block.begin() + i - 1);
								}
								block[i]->block_.back()->type_ = statement::type::t_empty;
								break;
							}
						}
						else if (previous_block
							&& previous_block->block_[previous_block->index_]->type_ == statement::type::t_if
							&& !function_in.is_valid_label(block[i]->instruction_.label_))
						{
							if (function_in.is_valid_label(previous_block->block_[previous_block->index_]->instruction_.label_)) {
								if (function_in.labels_[previous_block->block_[previous_block->index_]->instruction_.label_].target_
									== block[i]->block_.back()->instruction_.target_)
								{
									function_in.remove_jump(block[i]->block_.back()->instruction_.id_, block[i]->block_.back()->instruction_.target_);
									block[i]->type_ = statement::type::t_while;
									block[i]->assignment_.expressions_.emplace_back(previous_block->block_[previous_block->index_]->assignment_
										.expressions_.back());
									block[i]->instruction_.label_ = previous_block->block_[previous_block->index_]->instruction_.label_;
									block[i]->block_.back()->type_ = statement::type::t_empty;
									previous_block->block_[previous_block->index_] = block[i];
								}
							}
							else {
								for (auto j = previous_block->index_ - 1; j-- && previous_block->block_[j]->type_ == statement::type::t_if
									&& !previous_block->block_[j]->block_.size();)
								{
									if (!function_in.is_valid_label(previous_block->block_[j]->instruction_.label_)) {
										continue;
									}
									if (function_in.labels_[previous_block->block_[j]->instruction_.label_].target_
										!= block[i]->block_.back()->instruction_.target_)
									{
										break;
									}
									function_in.remove_jump(block[i]->block_.back()->instruction_.id_, block[i]->block_.back()->instruction_.target_);
									block[i]->type_ = statement::type::t_while;
									block[i]->assignment_.expressions_.resize(1, this->new_expression(ENUM_UNDER(expression::type::t_binary_operation)));
									block[i]->assignment_.expressions_.back()->binary_operation_->type_ = binary_operation::type::t_and;
									block[i]->assignment_.expressions_.back()->binary_operation_->right_operand_
										= previous_block->block_[previous_block->index_]->assignment_.expressions_.back();
									block[i]->instruction_.label_ = previous_block->block_[j]->instruction_.label_;
									previous_block->block_[j]->instruction_.label_ = INVALID_ID;

									for (expression* expr; j != previous_block->index_ - 1; j++) {
										expr = this->new_expression(ENUM_UNDER(expression::type::t_binary_operation));
										expr->binary_operation_->type_ = binary_operation::type::t_or;
										expr->binary_operation_->right_operand_ = this->new_primitive(2);
										expr->binary_operation_->left_operand_ = previous_block->block_[j]->assignment_.expressions_.back();

										if (block[i]->assignment_.expressions_.back()->binary_operation_->left_operand_) {
											previous_block->block_[j]->assignment_.expressions_.back()
												= this->new_expression(ENUM_UNDER(expression::type::t_binary_operation));
											previous_block->block_[j]->assignment_.expressions_.back()->binary_operation_->type_
												= binary_operation::type::t_and;
											previous_block->block_[j]->assignment_.expressions_.back()->binary_operation_->right_operand_ = expr;
											previous_block->block_[j]->assignment_.expressions_.back()->binary_operation_->left_operand_
												= block[i]->assignment_.expressions_.back()->binary_operation_->left_operand_;
											block[i]->assignment_.expressions_.back()->binary_operation_->left_operand_
												= previous_block->block_[j]->assignment_.expressions_.back();
										}
										else {
											block[i]->assignment_.expressions_.back()->binary_operation_->left_operand_ = expr;
										}
										previous_block->block_[j]->type_ = statement::type::t_empty;
									}
									block[i]->block_.back()->type_ = statement::type::t_empty;
									previous_block->block_[previous_block->index_] = block[i];
									break;
								}
							}
						}
					}
					if (block[i]->type_ == statement::type::t_loop) {
						block[i]->type_ = statement::type::t_repeat;
						block[i]->assignment_.expressions_.emplace_back(this->new_primitive(2));
					}
				}
				this->clean_up_block(function_in, block[i]->block_, variable_counter, iterator_counter, nullptr);
				continue;
			case statement::type::t_break:
				function_in.remove_jump(block[i]->instruction_.id_, block[i]->instruction_.target_);
				continue;
			case statement::type::t_declaration:
				while (block[i]->assignment_.expressions_.size()
					&& block[i]->assignment_.expressions_.back()->type_ == expression::type::t_constant
					&& block[i]->assignment_.expressions_.back()->constant_->type_ == constant::type::t_nil)
				{
					block[i]->assignment_.expressions_.pop_back();
				}
				for (auto j = block[i]->assignment_.variables_.size(); j--;) {
					(*block[i]->assignment_.variables_[j].slot_scope_)->name_ = block[i]->locals_->names_[j];
				}
				this->clean_up_block(function_in, block[i]->block_, variable_counter, iterator_counter, nullptr);
				continue;
			case statement::type::t_assignment:
				if (block[i]->assignment_.variables_.size() == 1
					&& block[i]->assignment_.variables_.back().type_ == variable::type::t_slot
					&& !(*block[i]->assignment_.variables_.back().slot_scope_)->usages_
					&& block[i]->assignment_.expressions_.size() == 1
					&& block[i]->assignment_.expressions_.back()->type_ == expression::type::t_table
					&& block[i]->assignment_.expressions_.back()->table_->fields_.size() == 1
					&& !block[i]->assignment_.expressions_.back()->table_->constants_.list_.size()
					&& !block[i]->assignment_.expressions_.back()->table_->constants_.fields_.size()
					&& !block[i]->assignment_.expressions_.back()->table_->multres_field_)
				{
					function_in.slot_scope_collector_.remove_scope(block[i]->assignment_.variables_.back().slot_,
						block[i]->assignment_.variables_.back().slot_scope_);
					block[i]->assignment_.variables_.back().type_ = variable::type::t_table_index;
					block[i]->assignment_.variables_.back().table_ = block[i]->assignment_.expressions_.back();
					block[i]->assignment_.variables_.back().table_index_ = block[i]->assignment_.expressions_.back()->table_->fields_.back().key_;
					block[i]->assignment_.expressions_.back() = block[i]->assignment_.expressions_.back()->table_->fields_.back().value_;
					block[i]->assignment_.variables_.back().table_->table_->fields_.pop_back();
					continue;
				}

				for (auto j = 0; j < block[i]->assignment_.variables_.size(); j++) {
					if (block[i]->assignment_.variables_[j].type_ != variable::type::t_slot
						|| (*block[i]->assignment_.variables_[j].slot_scope_)->name_.size())
					{
						block[i]->assignment_.forward_declaration_ = true;
						continue;
					}
					declarations.emplace_back(&block[i]->assignment_.variables_[j]);
					(*block[i]->assignment_.variables_[j].slot_scope_)->name_ = "var_"
						+ std::to_string(this->minimize_diffs_ ? function_in.level_ : function_in.id_) + "_"
						+ std::to_string(variable_counter);
					variable_counter++;
				}

				if (declarations.size()) {
					block.emplace(block.begin() + i, nullptr);
					i++;

					for (auto j = 0; j < declarations.size(); j++) {
						declarationTarget = &block[i - 1];

						for (block_info* currentBlockInfo = &blockInfo; currentBlockInfo->previous_block_;) {
							currentBlockInfo = currentBlockInfo->previous_block_;
							if (currentBlockInfo->index_ == currentBlockInfo->block_.size() - 1) {
								continue;
							}

							switch (currentBlockInfo->block_[currentBlockInfo->index_]->type_) {
							case statement::type::t_if:
								if (currentBlockInfo->block_[currentBlockInfo->index_ + 1]->type_ == statement::type::t_else) {
									if ((*declarations[j]->slot_scope_)->scope_end_
										<= currentBlockInfo->block_[currentBlockInfo->index_]->block_.back()->instruction_.id_)
									{
										break;
									}
									declarationTarget = &currentBlockInfo->block_[currentBlockInfo->index_ - 1];
									block[i]->assignment_.forward_declaration_ = true;
									continue;
								}
							case statement::type::t_else:
								switch (currentBlockInfo->block_[currentBlockInfo->index_ + 1]->type_) {
								case statement::type::t_empty:
								case statement::type::t_goto:
								case statement::type::t_break:
									if (currentBlockInfo->block_[currentBlockInfo->index_ + 1]->instruction_.type_ == bytecode::opcode::op_jmp) {
										if ((*declarations[j]->slot_scope_)->scope_end_
											< currentBlockInfo->block_[currentBlockInfo->index_ + 1]->instruction_.id_)
										{
											break;
										}
										declarationTarget = &currentBlockInfo->block_[currentBlockInfo->index_
											- (currentBlockInfo->block_[currentBlockInfo->index_]->type_ == statement::type::t_else ? 2 : 1)];
										block[i]->assignment_.forward_declaration_ = true;
										continue;
									}
								default:
									if ((*declarations[j]->slot_scope_)->scope_end_
										< function_in.labels_[currentBlockInfo->block_[currentBlockInfo->index_ + 1]->instruction_.label_].target_)
									{
										break;
									}
									declarationTarget = &currentBlockInfo->block_[currentBlockInfo->index_
										- (currentBlockInfo->block_[currentBlockInfo->index_]->type_ == statement::type::t_else ? 2 : 1)];
									block[i]->assignment_.forward_declaration_ = true;
									continue;
								}
								break;
							default:
								break;
							}
							break;
						}

						if (!*declarationTarget) {
							*declarationTarget = this->new_statement(ENUM_UNDER(statement::type::t_declaration));
							(*declarationTarget)->assignment_.forward_declaration_ = true;
							(*declarationTarget)->instruction_.target_ = (*declarations[j]->slot_scope_)->scope_begin_;
						}

						(*declarationTarget)->assignment_.variables_.emplace_back(*declarations[j]);
					}
					if (!block[i]->assignment_.forward_declaration_) {
						block[i]->type_ = statement::type::t_declaration;
						block[i]->assignment_.forward_declaration_ = true;
						block[i]->instruction_.target_ = block[i - 1]->instruction_.target_;
						block[i - 1] = nullptr;
						while (block[i]->assignment_.expressions_.size()
							&& block[i]->assignment_.expressions_.back()->type_ == expression::type::t_constant
							&& block[i]->assignment_.expressions_.back()->constant_->type_ == constant::type::t_nil)
						{
							block[i]->assignment_.expressions_.pop_back();
						}
					}
					if (!block[i - 1]) {
						i--;
						block.erase(block.begin() + i);
					}
					declarations.clear();
				}
				if (block[i]->type_ == statement::type::t_assignment) {
					while (block[i]->assignment_.expressions_.size() > 1
						&& block[i]->assignment_.expressions_.back()->type_ == expression::type::t_constant
						&& block[i]->assignment_.expressions_.back()->constant_->type_ == constant::type::t_nil)
					{
						block[i]->assignment_.expressions_.pop_back();
					}
				}
				continue;
			case statement::type::t_if:
				block.emplace(block.begin() + i, nullptr);
				i++;
				blockInfo.index_ = i;
				this->clean_up_block(function_in, block[i]->block_, variable_counter, iterator_counter, &blockInfo);
				if (!block[i]->block_.size()
					&& block[i]->assignment_.expressions_.back()->type_ == expression::type::t_constant
					&& block[i]->assignment_.expressions_.back()->constant_->type_ == constant::type::t_false)
				{
					block[i]->type_ = statement::type::t_empty;
				}
				if (i != block.size() - 1 && block[i + 1]->type_ == statement::type::t_else) {
					i++;
					if (block[i - 1]->type_ == statement::type::t_empty) {
						block[i]->type_ = statement::type::t_empty;
						block.reserve(block.size() + block[i]->block_.size());
						block.insert(block.begin() + i + 1, block[i]->block_.begin(), block[i]->block_.begin() + block[i]->block_.size());
						block[i]->block_.clear();
						block[i]->block_.shrink_to_fit();
					}
					else {
						blockInfo.index_++;
						this->clean_up_block(function_in, block[i]->block_, variable_counter, iterator_counter, &blockInfo);
						blockInfo.index_--;
						if (!block[i]->block_.size()) {
							block[i]->type_ = statement::type::t_empty;
						}
					}
				}

				if (block[blockInfo.index_ - 1]) {
					block[blockInfo.index_ - 1]->instruction_.target_ = block[blockInfo.index_]->instruction_.id_;
					continue;
				}

				block.erase(block.begin() + blockInfo.index_ - 1);
				i--;
				continue;
			default:
				continue;
			}
		}

		for (auto i = 0; i < block.size(); i++) {
			if (function_in.is_valid_label(block[i]->instruction_.label_)) {
				block.emplace(block.begin() + i, this->new_statement(ENUM_UNDER(statement::type::t_label)));
				block[i]->instruction_.label_ = block[i + 1]->instruction_.label_;
				i++;
			}
			switch (block[i]->type_) {
			case statement::type::t_empty:
				block.erase(block.begin() + i);
				i--;
				continue;
			case statement::type::t_goto:
				block[i]->instruction_.label_ = function_in.get_label_from_id(block[i]->instruction_.target_);
				continue;
			default:
				continue;
			}
		}

		std::vector<std::uint32_t> labels;
		for (std::uint32_t i = block.size(); i--;) {
			if (block[i]->type_ != statement::type::t_declaration) {
				continue;
			}
			if (block[i]->assignment_.forward_declaration_) {
				for (auto j = i; j < block.size(); j++) {
					if (block[j]->type_ != statement::type::t_label) {
						continue;
					}
					if (function_in.labels_[block[j]->instruction_.label_].jump_ids_.front() < block[i]->instruction_.target_) {
						for (auto k = labels.size(); k--;) {
							if (function_in.labels_[block[labels[k]]->instruction_.label_].jump_ids_.back()
								>= function_in.labels_[block[j]->instruction_.label_].target_)
							{
								j = labels[k];
							}
						}
						block.emplace(block.begin() + i, this->new_statement(ENUM_UNDER(statement::type::t_do)));
						j++;
						block[i]->block_.reserve(j - 1 - i);
						block[i]->block_.insert(block[i]->block_.begin(), block.begin() + i + 1, block.begin() + j);
						block.erase(block.begin() + i + 1, block.begin() + j);
						if (block[i]->block_.size() && block[i]->block_.back()->type_ == statement::type::t_do) {
							block[i]->block_.reserve(block[i]->block_.size() + block[i]->block_.back()->block_.size());
							block[i]->block_.insert(block[i]->block_.begin() + block[i]->block_.size() - 1, block[i]->block_.back()->block_.begin(),
								block[i]->block_.back()->block_.begin() + block[i]->block_.back()->block_.size());
							block[i]->block_.back()->block_.clear();
							block[i]->block_.back()->block_.shrink_to_fit();
							block[i]->block_.pop_back();
						}
						break;
					}
					if (function_in.labels_[block[j]->instruction_.label_].jump_ids_.back() > function_in.labels_[block[j]->instruction_.label_].target_) {
						labels.emplace_back(j);
					}
				}
				labels.clear();
				continue;
			}
			if (i == block.size() - 1) {
				block.reserve(block.size() + block[i]->block_.size());
				block.insert(block.begin() + block.size(), block[i]->block_.begin(), block[i]->block_.begin() + block[i]->block_.size());
				block[i]->block_.clear();
				block[i]->block_.shrink_to_fit();
				continue;
			}
			block[i]->block_.emplace(block[i]->block_.begin(), this->new_statement(ENUM_UNDER(statement::type::t_declaration)));
			block[i]->block_.front()->assignment_.variables_ = block[i]->assignment_.variables_;
			block[i]->block_.front()->assignment_.expressions_ = block[i]->assignment_.expressions_;
			block[i]->type_ = statement::type::t_do;
		}
	}

	ast::expression* ast::new_slot(const std::uint8_t& slot) {
		expression* const expression = this->new_expression(ENUM_UNDER(ast::expression::type::t_variable));
		expression->variable_->type_ = ast::variable::type::t_slot;
		expression->variable_->slot_ = slot;
		return expression;
	}

	ast::expression* ast::new_literal(const std::uint8_t& literal) {
		expression* const expression = this->new_expression(ENUM_UNDER(ast::expression::type::t_constant));
		expression->constant_->type_ = ast::constant::type::t_number;
		expression->constant_->number_ = literal;
		return expression;
	}

	ast::expression* ast::new_signed_literal(const std::uint16_t& signed_literal) {
		expression* const expression = this->new_expression(ENUM_UNDER(ast::expression::type::t_constant));
		expression->constant_->type_ = ast::constant::type::t_number;
		expression->constant_->number_ = std::bit_cast<std::int16_t>(signed_literal);
		return expression;
	}

	ast::expression* ast::new_primitive(const std::uint8_t& primitive) {
		expression* const expression = this->new_expression(ENUM_UNDER(ast::expression::type::t_constant));
		switch (primitive) {
		case 0:
			expression->constant_->type_ = ast::constant::type::t_nil;
			break;
		case 1:
			expression->constant_->type_ = ast::constant::type::t_false;
			break;
		case 2:
			expression->constant_->type_ = ast::constant::type::t_true;
			break;
		default:
			throw nullptr;
		}
		return expression;
	}

	ast::expression* ast::new_number(const function& function_in, const std::uint16_t& index) {
		expression* const expression = this->new_expression(ENUM_UNDER(ast::expression::type::t_constant));
		expression->constant_->type_ = ast::constant::type::t_number;
		switch (function_in.get_number_constant(index).type_) {
		case bytecode::number_constant::type::t_int:
			expression->constant_->number_ = std::bit_cast<int>(function_in.get_number_constant(index).integer_);
			break;
		case bytecode::number_constant::type::t_num:
			expression->constant_->number_ = std::bit_cast<double>(function_in.get_number_constant(index).number_);
			this->check_special_number(expression);
			break;
		}
		return expression;
	}

	ast::expression* ast::new_string(const function& function_in, const std::uint16_t& index) {
		expression* const expression = this->new_expression(ENUM_UNDER(ast::expression::type::t_constant));
		expression->constant_->type_ = ast::constant::type::t_string;
		expression->constant_->string_ = function_in.get_constant(index).string_;
		return expression;
	}

	ast::expression* ast::new_table(const function& function_in, const std::uint16_t& index) {
		const auto new_table_constant = [this](const bytecode::table_constant& constant) -> expression* {
			expression* const expression = this->new_expression(ENUM_UNDER(expression::type::t_constant));
			switch (constant.type_) {
			case bytecode::table_constant::type::t_nil:
				expression->constant_->type_ = constant::type::t_nil;
				break;
			case bytecode::table_constant::type::t_false:
				expression->constant_->type_ = constant::type::t_false;
				break;
			case bytecode::table_constant::type::t_true:
				expression->constant_->type_ = constant::type::t_true;
				break;
			case bytecode::table_constant::type::t_int:
				expression->constant_->type_ = constant::type::t_number;
				expression->constant_->number_ = std::bit_cast<int>(constant.integer_);
				break;
			case bytecode::table_constant::type::t_num:
				expression->constant_->type_ = constant::type::t_number;
				expression->constant_->number_ = std::bit_cast<double>(constant.number_);
				this->check_special_number(expression);
				break;
			case bytecode::table_constant::type::t_xhash:
				expression->constant_->type_ = constant::type::t_xhash;
				expression->constant_->xhash_ = constant.xhash_;
				expression->constant_->xhash_type_ = constant.xhash_type_;
				break;
			case bytecode::table_constant::type::t_str:
				expression->constant_->type_ = constant::type::t_string;
				expression->constant_->string_ = constant.string_;
				break;
			}
			return expression;
		};

		expression* const expr = this->new_expression(ENUM_UNDER(expression::type::t_table));
		expr->table_->constants_.list_.resize(function_in.get_constant(index).array_.size(), nullptr);

		for (auto x = expr->table_->constants_.list_.size(); x--;) {
			expr->table_->constants_.list_[x] = new_table_constant(function_in.get_constant(index).array_[x]);
		}

		if (this->minimize_diffs_) {
			std::uint32_t position;
			expression* key;
			expression** value;
			table::field false_field, true_field;
			std::vector<table::field> number_fields, xhash_fields, string_fields;

			for (auto i = function_in.get_constant(index).table_.size(); i--;) {
				key = new_table_constant(function_in.get_constant(index).table_[i].key_);

				switch (key->constant_->type_) {
				case constant::type::t_false:
					false_field.key_ = key;
					value = &false_field.value_;
					break;
				case constant::type::t_true:
					true_field.key_ = key;
					value = &true_field.value_;
					break;
				case constant::type::t_number:
					position = number_fields.size();
					while (position && number_fields[position - 1].key_->constant_->number_ > key->constant_->number_) {
						position--;
					}
					number_fields.emplace(number_fields.begin() + position, table::field{
						.key_ = key
					});
					value = &number_fields[position].value_;
					break;
				case constant::type::t_xhash:
					position = xhash_fields.size();
					while (position && (xhash_fields[position - 1].key_->constant_->xhash_type_ > key->constant_->xhash_type_
						|| xhash_fields[position - 1].key_->constant_->xhash_ > key->constant_->xhash_))
					{
						position--;
					}
					xhash_fields.emplace(xhash_fields.begin() + position, table::field{
						.key_ = key
					});
					value = &xhash_fields[position].value_;
					break;
				case constant::type::t_string:
					this->check_valid_name(key->constant_);
					position = string_fields.size();
					while (position && string_fields[position - 1].key_->constant_->string_.compare(key->constant_->string_) > 0) {
						position--;
					}
					string_fields.emplace(string_fields.begin() + position, table::field{
						.key_ = key
					});
					value = &string_fields[position].value_;
					break;
				default:
					throw nullptr;
				}
				*value = new_table_constant(function_in.get_constant(index).table_[i].value_);
			}

			if (false_field.key_) {
				expr->table_->constants_.fields_.emplace_back(false_field);
			}
			if (true_field.key_) {
				expr->table_->constants_.fields_.emplace_back(true_field);
			}
			expr->table_->constants_.fields_.reserve(expr->table_->constants_.fields_.size()
				+ number_fields.size() + xhash_fields.size() + string_fields.size());
			expr->table_->constants_.fields_.insert(expr->table_->constants_.fields_.begin() + expr->table_->constants_.fields_.size(),
				number_fields.begin(), number_fields.begin() + number_fields.size());
			expr->table_->constants_.fields_.insert(expr->table_->constants_.fields_.begin() + expr->table_->constants_.fields_.size(),
				xhash_fields.begin(), xhash_fields.begin() + xhash_fields.size());
			expr->table_->constants_.fields_.insert(expr->table_->constants_.fields_.begin() + expr->table_->constants_.fields_.size(),
				string_fields.begin(), string_fields.begin() + string_fields.size());
		}
		else {
			expr->table_->constants_.fields_.resize(function_in.get_constant(index).table_.size());
			for (auto i = expr->table_->constants_.fields_.size(); i--;) {
				expr->table_->constants_.fields_[i].key_ = new_table_constant(function_in.get_constant(index).table_[i].key_);
				if (expr->table_->constants_.fields_[i].key_->constant_->type_ == constant::type::t_string) {
					check_valid_name(expr->table_->constants_.fields_[i].key_->constant_);
				}
				expr->table_->constants_.fields_[i].value_ = new_table_constant(function_in.get_constant(index).table_[i].value_);
			}
		}

		return expr;
	}

	ast::expression* ast::new_cdata(const function& function_in, const std::uint16_t& index) {
		expression* const expression = this->new_expression(ENUM_UNDER(ast::expression::type::t_constant));
		switch (function_in.get_constant(index).type_) {
		case bytecode::constant::type::t_i64:
			expression->constant_->type_ = constant::type::t_cdata_signed;
			expression->constant_->int_signed_ = std::bit_cast<std::int64_t>(function_in.get_constant(index).cdata_);
			break;
		case bytecode::constant::type::t_u64:
			expression->constant_->type_ = constant::type::t_cdata_unsigned;
			expression->constant_->int_unsigned_ = function_in.get_constant(index).cdata_;
			break;
		case bytecode::constant::type::t_complex:
			expression->constant_->type_ = constant::type::t_cdata_imaginary;
			expression->constant_->number_ = std::bit_cast<double>(function_in.get_constant(index).cdata_);
			this->check_special_number(expression, true);
			break;
		default:
			break;
		}
		return expression;
	}

	ast::expression* ast::new_xhash(const function& function_in, const std::uint16_t& index) {
		expression* const expression = this->new_expression(ENUM_UNDER(ast::expression::type::t_constant));
		expression->constant_->type_ = constant::type::t_xhash;
		expression->constant_->xhash_ = function_in.get_constant(index).xhash_;
		expression->constant_->xhash_type_ = function_in.get_constant(index).xhash_type_;
		return expression;
	}

	void ast::check_special_number(expression* const& expression_in, const bool& is_cdata) {
		const std::uint64_t raw_double = std::bit_cast<std::uint64_t>(expression_in->constant_->number_);
		if ((raw_double & DOUBLE_EXPONENT) != DOUBLE_SPECIAL) {
			ld_assert(raw_double != DOUBLE_NEGATIVE_ZERO || is_cdata, "Number constant is negative zero", this->bc_.file_path_);
			return;
		}
		ld_assert(!(raw_double & DOUBLE_FRACTION), "Number constant is NaN", this->bc_.file_path_);
		if (is_cdata) {
			return;
		}
		expression_in->set_type(expression::type::t_binary_operation);
		expression_in->binary_operation_->type_ = binary_operation::type::t_division;
		expression_in->binary_operation_->left_operand_ = this->new_expression(ENUM_UNDER(expression::type::t_constant));
		expression_in->binary_operation_->left_operand_->constant_->type_ = ast::constant::type::t_number;
		expression_in->binary_operation_->left_operand_->constant_->number_ = raw_double & DOUBLE_SIGN ? -1 : 1;
		expression_in->binary_operation_->right_operand_ = this->new_expression(ENUM_UNDER(expression::type::t_constant));
		expression_in->binary_operation_->right_operand_->constant_->type_ = ast::constant::type::t_number;
		expression_in->binary_operation_->right_operand_->constant_->number_ = 0;
	}
}

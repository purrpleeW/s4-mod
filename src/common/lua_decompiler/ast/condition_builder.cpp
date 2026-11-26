#include "common_core.hpp"
#include "lua_decompiler/ast/condition_builder.hpp"

#include "lua_decompiler/ast/binary_operation.hpp"
#include "lua_decompiler/ast/unary_operation.hpp"

namespace lua_decompiler {
	ast::condition_builder::node::type ast::condition_builder::get_node_type(const bytecode::opcode& instruction, const bool& swapped) {
		switch (instruction) {
		case bytecode::opcode::op_islt:
			return swapped
				? node::type::t_greater_than
				: node::type::t_less_than;
		case bytecode::opcode::op_isge:
			return swapped
				? node::type::t_not_greater_than
				: node::type::t_not_less_than;
		case bytecode::opcode::op_isle:
			return swapped
				? node::type::t_greater_equal
				: node::type::t_less_equal;
		case bytecode::opcode::op_isgt:
			return swapped
				? node::type::t_not_greater_equal
				: node::type::t_not_less_equal;
		case bytecode::opcode::op_iseqv:
		case bytecode::opcode::op_iseqs:
		case bytecode::opcode::op_iseqn:
		case bytecode::opcode::op_iseqp:
			return node::type::t_equal;
		case bytecode::opcode::op_isnev:
		case bytecode::opcode::op_isnes:
		case bytecode::opcode::op_isnen:
		case bytecode::opcode::op_isnep:
			return node::type::t_not_equal;
		case bytecode::opcode::op_istc:
		case bytecode::opcode::op_ist:
			return node::type::t_truthy_test;
		case bytecode::opcode::op_isfc:
		case bytecode::opcode::op_isf:
			return node::type::t_falsy_test;
		case bytecode::opcode::op_jmp:
			return node::type::t_unconditional;
		default:
			throw nullptr;
		}
	}

	bool ast::condition_builder::invert_any_node(node* const& left_node, node* const& right_node) {
		auto end_target = node::type::t_end_target;
		auto type_pref = node::type_preference;
		if (ENUM_UNDER(left_node->target_node_->type_) < ENUM_UNDER(end_target)) {
			invert_node(ENUM_UNDER(right_node->target_node_->type_) > ENUM_UNDER(end_target) || type_pref[ENUM_UNDER(left_node->type_)][!left_node->inverted_]
					>= type_pref[ENUM_UNDER(right_node->type_)][!right_node->inverted_]
				? left_node : right_node);
		}
		else {
			if (ENUM_UNDER(right_node->target_node_->type_) > ENUM_UNDER(end_target)) {
				return false;
			}
			invert_node(right_node);
		}
		return true;
	}

	void ast::condition_builder::invert_node(node* const& node_in) {
		node_in->inverted_ = !node_in->inverted_;
		if (node::type_preference[ENUM_UNDER(node_in->type_)][node_in->inverted_]) {
			return;
		}
		invert_node(node_in->left_node_);
		invert_node(node_in->right_node_);
		node_in->type_ = node_in->inverted_
			? (node_in->type_ == node::type::t_and ? node::type::t_not_or : node::type::t_not_and)
			: (node_in->type_ == node::type::t_not_and ? node::type::t_or : node::type::t_and);
	}

	ast::condition_builder::node*& ast::condition_builder::new_node(const node::type& type_in) {
		return this->nodes_.emplace_back(new node(type_in));
	}

	void ast::condition_builder::add_node(const node::type& type_in, const std::uint32_t& node_label, const std::uint32_t& target_label,
		std::vector<expression*>* const& expressions)
	{
		this->condition_nodes_.emplace_back(this->new_node(type_in));
		this->condition_nodes_.back()->node_label_ = node_label;
		this->condition_nodes_.back()->target_label_ = target_label;
		this->condition_nodes_.back()->expressions_ = expressions;
	}

	bool ast::condition_builder::link_nodes() {
		switch (this->type_) {
		case type::t_assignment:
			this->condition_nodes_.emplace_back(this->false_target_);
			this->condition_nodes_.emplace_back(this->true_target_);
			this->condition_nodes_.emplace_back(this->end_target_);
			break;
		case type::t_statement:
			this->condition_nodes_.emplace_back(this->true_target_);
			this->condition_nodes_.emplace_back(this->false_target_);
			break;
		}

		for (auto x = this->condition_nodes_.size(); x--;) {
			if (this->condition_nodes_[x]->target_label_ == INVALID_ID) {
				continue;
			}
			for (auto y = this->condition_nodes_.size(); y--;) {
				if (this->condition_nodes_[x]->target_label_ != this->condition_nodes_[y]->node_label_) {
					continue;
				}
				this->condition_nodes_[x]->target_node_ = this->condition_nodes_[y];
				this->condition_nodes_[y]->incoming_nodes_++;
				break;
			}
			if (!this->condition_nodes_[x]->target_node_) {
				return false;
			}
		}

		this->condition_nodes_.pop_back();
		if (this->type_ == type::t_assignment) {
			this->condition_nodes_.pop_back();
		}
		return true;
	}

	void ast::condition_builder::fix_return_nodes() {
		switch (this->type_) {
		case type::t_assignment:
			for (auto x = this->condition_nodes_.size() - 1; x--;) {
				switch (this->condition_nodes_[x]->target_node_->type_) {
				case node::type::t_end_target:
					this->condition_nodes_[x]->target_node_ = this->condition_nodes_[x]->type_ == node::type::t_truthy_test
						? this->true_target_ : this->false_target_;
					this->condition_nodes_[x]->target_node_->incoming_nodes_++;
					this->condition_nodes_[x]->inverted_ = this->condition_nodes_[x]->type_ == node::type::t_falsy_test;
					continue;
				case node::type::t_true_target:
					if (this->condition_nodes_[x]->type_ == node::type::t_truthy_test) {
						this->condition_nodes_[x]->type_ = node::type::t_bool_truthy_test;
					}
					continue;
				case node::type::t_false_target:
					if (this->condition_nodes_[x]->type_ == node::type::t_falsy_test) {
						this->condition_nodes_[x]->type_ = node::type::t_bool_falsy_test;
					}
					this->condition_nodes_[x]->inverted_ = true;
					continue;
				default:
					continue;
				}
			}
			break;
		case type::t_statement:
			for (auto x = this->condition_nodes_.size() - 1; x--;) {
				if (this->condition_nodes_[x]->target_node_->type_ == node::type::t_false_target) {
					this->condition_nodes_[x]->inverted_ = true;
				}
			}
#			define x (this->condition_nodes_.size() - 2)
			if (this->condition_nodes_[x]->inverted_) {
				break;
			}
			this->condition_nodes_[x]->left_node_ = this->copy_node(this->condition_nodes_[x]);
			this->condition_nodes_[x]->right_node_ = this->new_node(node::type::t_unconditional);
			this->condition_nodes_[x]->target_node_->incoming_nodes_--;
			this->condition_nodes_[x]->target_node_ = this->false_target_;
			this->condition_nodes_[x]->target_node_->incoming_nodes_++;
			this->condition_nodes_[x]->type_ = node::type::t_not_or;
			this->condition_nodes_[x]->inverted_ = true;
#			undef x
			break;
		}
	}

	bool ast::condition_builder::build_boolean_logic() {
		for (auto x = this->condition_nodes_.size() - 1; --x;) {
			if (!this->condition_nodes_[x]->incoming_nodes_) {
				if (this->condition_nodes_[x - 1]->target_node_ == this->condition_nodes_[x + 1]) {
					if (this->condition_nodes_[x - 1]->inverted_ == this->condition_nodes_[x]->inverted_
						&& !this->invert_any_node(this->condition_nodes_[x - 1], this->condition_nodes_[x]))
					{
						return false;
					}
					this->merge_nodes(this->condition_nodes_[x - 1], this->condition_nodes_[x]);
					this->condition_nodes_[x - 1]->type_ = this->condition_nodes_[x - 1]->inverted_ ? node::type::t_not_or : node::type::t_and;
					this->condition_nodes_.erase(this->condition_nodes_.begin() + x);
					x = this->condition_nodes_.size() - 1;
				}
				else if (this->condition_nodes_[x - 1]->target_node_ == this->condition_nodes_[x]->target_node_) {
					if (this->condition_nodes_[x - 1]->inverted_ != this->condition_nodes_[x]->inverted_
						&& !this->invert_any_node(this->condition_nodes_[x - 1], this->condition_nodes_[x]))
					{
						return false;
					}
					this->merge_nodes(this->condition_nodes_[x - 1], this->condition_nodes_[x]);
					this->condition_nodes_[x - 1]->type_ = this->condition_nodes_[x - 1]->inverted_ ? node::type::t_not_and : node::type::t_or;
					this->condition_nodes_.erase(this->condition_nodes_.begin() + x);
					x = this->condition_nodes_.size() - 1;
				}
				else if (this->condition_nodes_[x]->target_node_ == this->condition_nodes_[x + 1]) {
					this->condition_nodes_[x]->left_node_ = this->copy_node(this->condition_nodes_[x]);
					this->condition_nodes_[x]->right_node_ = this->new_node(node::type::t_unconditional);
					this->condition_nodes_[x]->inverted_ = this->condition_nodes_[x - 1]->inverted_;
					if (this->condition_nodes_[x]->left_node_->inverted_ == this->condition_nodes_[x]->inverted_) {
						this->invert_node(this->condition_nodes_[x]->left_node_);
					}
					this->condition_nodes_[x]->right_node_->inverted_ = !this->condition_nodes_[x]->inverted_;
					this->condition_nodes_[x]->type_ = this->condition_nodes_[x]->inverted_ ? node::type::t_not_or : node::type::t_and;
					std::swap<node*>(this->condition_nodes_[x]->target_node_, this->condition_nodes_[x - 1]->target_node_);
					this->merge_nodes(this->condition_nodes_[x - 1], this->condition_nodes_[x]);
					this->condition_nodes_[x - 1]->type_ = this->condition_nodes_[x - 1]->inverted_ ? node::type::t_not_and : node::type::t_or;
					this->condition_nodes_.erase(this->condition_nodes_.begin() + x);
					x = this->condition_nodes_.size() - 1;
				}
			}
			else if (this->condition_nodes_[x]->incoming_nodes_ == 1 && this->condition_nodes_[x - 1]->target_node_ == this->condition_nodes_[x]) {
				if (node::type_preference[ENUM_UNDER(this->condition_nodes_[x - 1]->type_)][this->condition_nodes_[x - 1]->inverted_] != 3) {
					this->invert_node(this->condition_nodes_[x - 1]);
				}
				this->condition_nodes_[x - 1]->left_node_ = this->copy_node(this->condition_nodes_[x - 1]);
				this->condition_nodes_[x - 1]->right_node_ = this->new_node(node::type::t_unconditional);
				this->condition_nodes_[x - 1]->right_node_->inverted_ = this->condition_nodes_[x - 1]->inverted_;
				this->condition_nodes_[x - 1]->type_ = this->condition_nodes_[x - 1]->inverted_ ? node::type::t_not_and : node::type::t_or;
				merge_nodes(this->condition_nodes_[x - 1], this->condition_nodes_[x]);
				this->condition_nodes_[x - 1]->type_ = this->condition_nodes_[x - 1]->left_node_->inverted_
					? (this->condition_nodes_[x - 1]->inverted_ ? node::type::t_not_or : node::type::t_or)
					: (this->condition_nodes_[x - 1]->inverted_ ? node::type::t_not_and : node::type::t_and);
				this->condition_nodes_.erase(this->condition_nodes_.begin() + x);
				x = this->condition_nodes_.size() - 1;
			}
		}
		this->condition_nodes_.pop_back();
		return this->condition_nodes_.size() == 1;
	}

	ast::condition_builder::node* ast::condition_builder::copy_node(node* const& node_in) {
		node* const copy = this->new_node(node_in->type_);
		copy->inverted_ = node_in->inverted_;
		copy->expressions_ = node_in->expressions_;
		copy->left_node_ = node_in->left_node_;
		copy->right_node_ = node_in->right_node_;
		return copy;
	}

	void ast::condition_builder::merge_nodes(node* const& node_in, node* const& target_node) {
		node_in->left_node_ = this->copy_node(node_in);
		node_in->right_node_ = target_node;
		node_in->target_node_->incoming_nodes_--;
		node_in->target_node_ = target_node->target_node_;
		node_in->inverted_ = target_node->inverted_;
	}

	ast::expression* ast::condition_builder::build_expression(node* const& node_in) {
		static auto build_consistent = [](ast::condition_builder* b, node* const& n, expression* e, bool o) -> auto {
			if ((n->inverted_ && !o) || (!n->inverted_ && o)) {
				return b->build_not(e);
			}
			return e;
		};

		switch (node_in->type_) {
		case node::type::t_less_than:
		case node::type::t_less_equal:
		case node::type::t_greater_than:
		case node::type::t_greater_equal:
			return build_consistent(this, node_in, this->build_binary(node_in->type_, (*node_in->expressions_)[0], (*node_in->expressions_)[1]), false);
		case node::type::t_not_less_than:
		case node::type::t_not_less_equal:
		case node::type::t_not_greater_than:
		case node::type::t_not_greater_equal:
			return build_consistent(this, node_in, this->build_binary(node_in->type_, (*node_in->expressions_)[0], (*node_in->expressions_)[1]), true);
		case node::type::t_equal:
			return this->build_binary(node_in->inverted_ ? node::type::t_not_equal : node::type::t_equal, (*node_in->expressions_)[0],
				(*node_in->expressions_)[1]);
		case node::type::t_not_equal:
			return this->build_binary(node_in->inverted_ ? node::type::t_equal : node::type::t_not_equal, (*node_in->expressions_)[0],
				(*node_in->expressions_)[1]);
		case node::type::t_truthy_test:
			return build_consistent(this, node_in, (*node_in->expressions_).back(), false);
		case node::type::t_falsy_test:
			return build_consistent(this, node_in, (*node_in->expressions_).back(), true);
		case node::type::t_bool_truthy_test:
			return build_consistent(this, node_in, this->build_not((*node_in->expressions_).back()), true);
		case node::type::t_bool_falsy_test:
			return build_consistent(this, node_in, this->build_not((*node_in->expressions_).back()), false);
		case node::type::t_unconditional:
			return this->ast_.new_primitive(node_in->inverted_ ? 1 : 2);
		case node::type::t_and:
		case node::type::t_or:
			return build_consistent(this, node_in, this->build_binary(node_in->type_, this->build_expression(node_in->left_node_),
				this->build_expression(node_in->right_node_)), false);
		case node::type::t_not_and:
		case node::type::t_not_or:
			return build_consistent(this, node_in, this->build_binary(node_in->type_, this->build_expression(node_in->left_node_),
				this->build_expression(node_in->right_node_)), true);
		default:
			throw nullptr;
		}
	}

	ast::expression* ast::condition_builder::build_not(expression* const& operand) {
		expression* const expression = this->ast_.new_expression(ENUM_UNDER(expression::type::t_unary_operation));
		expression->unary_operation_->type_ = unary_operation::type::t_not;
		expression->unary_operation_->operand_ = operand;
		return expression;
	}

	ast::expression* ast::condition_builder::build_binary(const node::type& type_in, expression* const& left_operand, expression* const& right_operand) {
		expression* const expression = this->ast_.new_expression(ENUM_UNDER(expression::type::t_binary_operation));
		switch (type_in) {
		case node::type::t_less_than:
		case node::type::t_not_less_than:
			expression->binary_operation_->type_ = binary_operation::type::t_less_than;
			break;
		case node::type::t_less_equal:
		case node::type::t_not_less_equal:
			expression->binary_operation_->type_ = binary_operation::type::t_less_equal;
			break;
		case node::type::t_greater_than:
		case node::type::t_not_greater_than:
			expression->binary_operation_->type_ = binary_operation::type::t_greater_than;
			break;
		case node::type::t_greater_equal:
		case node::type::t_not_greater_equal:
			expression->binary_operation_->type_ = binary_operation::type::t_greater_equal;
			break;
		case node::type::t_equal:
			expression->binary_operation_->type_ = binary_operation::type::t_equal;
			break;
		case node::type::t_not_equal:
			expression->binary_operation_->type_ = binary_operation::type::t_not_equal;
			break;
		case node::type::t_and:
		case node::type::t_not_and:
			expression->binary_operation_->type_ = binary_operation::type::t_and;
			break;
		case node::type::t_or:
		case node::type::t_not_or:
			expression->binary_operation_->type_ = binary_operation::type::t_or;
			break;
		default:
			throw nullptr;
		}
		expression->binary_operation_->left_operand_ = left_operand;
		expression->binary_operation_->right_operand_ = right_operand;
		return expression;
	}

	ast::expression* ast::condition_builder::build_condition() {
		if (this->link_nodes()) {
			this->fix_return_nodes();
			if (this->build_boolean_logic()) {
				return this->build_expression(this->condition_nodes_.back());
			}
		}
		return nullptr;
	}
}

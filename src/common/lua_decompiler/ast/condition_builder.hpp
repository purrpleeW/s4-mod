#pragma once
#include "common_core.hpp"
#include "lua_decompiler/ast_impl.hpp"

#include "lua_decompiler/ast/expression.hpp"
#include "lua_decompiler/bytecode/opcode.hpp"

namespace lua_decompiler {
	struct ast::condition_builder {
		struct node {
			enum class type {
				t_less_than,
				t_less_equal,
				t_greater_than,
				t_greater_equal,
				t_not_less_than,
				t_not_less_equal,
				t_not_greater_than,
				t_not_greater_equal,
				t_equal,
				t_not_equal,
				t_truthy_test,
				t_falsy_test,
				t_bool_truthy_test,
				t_bool_falsy_test,
				t_unconditional,
				t_and,
				t_or,
				t_not_and,
				t_not_or,
				t_end_target,
				t_true_target,
				t_false_target
			};

			static constexpr std::uint8_t type_preference[ENUM_UNDER(type::t_end_target)][2] = {
				{ 3, 1 },	// t_less_than
				{ 3, 1 },	// t_less_equal
				{ 3, 1 },	// t_greater_than
				{ 3, 1 },	// t_greater_equal
				{ 1, 3 },	// t_not_less_than
				{ 1, 3 },	// t_not_less_equal
				{ 1, 3 },	// t_not_greater_than
				{ 1, 3 },	// t_not_greater_equal
				{ 3, 3 },	// t_equal
				{ 3, 3 },	// t_not_equal
				{ 3, 2 },	// t_truthy_test
				{ 2, 3 },	// t_falsy_test
				{ 0, 0 },	// t_bool_truthy_test
				{ 0, 0 },	// t_bool_falsy_test
				{ 3, 3 },	// t_unconditional
				{ 3, 0 },	// t_and
				{ 3, 0 },	// t_or
				{ 0, 3 },	// t_not_and
				{ 0, 3 }	// t_not_or
			};

			node(const type& type_in)
				: type_(type_in)
			{}

			type type_;
			std::uint32_t node_label_ = INVALID_ID;
			std::uint32_t target_label_ = INVALID_ID;
			node* target_node_ = nullptr;
			std::uint32_t incoming_nodes_ = 0;
			bool inverted_ = false;
			std::vector<expression*>* expressions_ = nullptr;
			node* left_node_ = nullptr;
			node* right_node_ = nullptr;
		};

		enum class type {
			t_assignment,
			t_statement
		};

		condition_builder(const type& type_in, ast& ast_in, const std::uint32_t& end_target_label, const std::uint32_t& true_target_label,
			const std::uint32_t& false_target_label
		)
			: type_(type_in), ast_(ast_in)
		{
			if (this->type_ == type::t_assignment) {
				this->end_target_ = this->new_node(node::type::t_end_target);
				this->end_target_->node_label_ = end_target_label;
			}
			this->true_target_ = this->new_node(node::type::t_true_target);
			this->true_target_->node_label_ = true_target_label;
			this->false_target_ = this->new_node(node::type::t_false_target);
			this->false_target_->node_label_ = false_target_label;
		}

		~condition_builder() {
			for (auto x = this->nodes_.size(); x--;) {
				delete this->nodes_[x];
			}
		}

		static node::type get_node_type(const bytecode::opcode& instruction, const bool& swapped);
		static bool invert_any_node(node* const& left_node, node* const& right_node);
		static void invert_node(node* const& node_in);

		node*& new_node(const node::type& type_in);
		void add_node(const node::type& type_in, const std::uint32_t& node_label, const std::uint32_t& target_label,
			std::vector<expression*>* const& expressions);
		bool link_nodes();
		void fix_return_nodes();
		bool build_boolean_logic();
		node* copy_node(node* const& node_in);
		void merge_nodes(node* const& node_in, node* const& target_node);
		expression* build_expression(node* const& node_in);
		expression* build_not(expression* const& operand);
		expression* build_binary(const node::type& type_in, expression* const& left_operand, expression* const& right_operand);
		expression* build_condition();

		type type_;
		ast& ast_;
		std::vector<node*> nodes_{};
		std::vector<node*> condition_nodes_{};
		node* end_target_ = nullptr;
		node* true_target_ = nullptr;
		node* false_target_ = nullptr;
	};
}

#pragma once
#include "common_core.hpp"
#include "lua_decompiler/bytecode_impl.hpp"

namespace lua_decompiler {
	class ast {
	public:
		struct binary_operation;
		struct condition_builder;
		struct constant;
		struct expression;
		struct function;
		struct function_call;
		struct local;
		struct slot_scope;
		struct statement;
		struct table;
		struct unary_operation;
		struct variable;

		enum class constant_type {
			t_invalid,
			t_nil,
			t_bool,
			t_number
		};

		struct block_info {
			std::uint32_t index_ = INVALID_ID;
			std::vector<statement*>& block_;
			block_info* const previous_block_;
		};

		static constexpr std::uint32_t INVALID_ID = -1;

		ast(const bytecode& bc, const bool& ignore_debug_info, const bool& minimize_diffs);
		~ast();

		static std::uint32_t get_block_index_from_id(const std::vector<statement*>& block, const std::uint32_t& id);
		static std::uint32_t get_extended_id_from_statement(statement* const& statement_in);
		static std::uint32_t get_label_from_next_statement(function& function_in, const block_info& block_info_in, const bool& return_extended_label,
			const bool& exclude_declaration);
		static bool is_valid_block(function& function_in, const block_info& block_info_in, const std::uint32_t& block_begin);
		static void check_valid_name(constant* const& constant_in);
		static constant_type get_constant_type(expression* const& expression_in);

		void operator()();
		function*& new_function(const bytecode::prototype& prototype, const std::uint32_t& level);
		statement*& new_statement(const int& type);
		expression*& new_expression(const int& type);
		void build_functions(function& function_in, std::uint32_t& function_counter);
		void build_instructions(function& function_in);
		void assign_debug_info(function& function_in);
		void group_jumps(function& function_in);
		void build_loops(function& function_in);
		void build_local_scopes(function& function_in, std::vector<statement*>& block);
		void build_expressions(function& function_in, std::vector<statement*>& block);
		void build_slot_scopes(function& function_in, std::vector<statement*>& block, block_info* const& previous_block);
		void eliminate_slots(function& function_in, std::vector<statement*>& block, block_info* const& previous_block);
		void eliminate_conditions(function& function_in, std::vector<statement*>& block, block_info* const& previous_block);
		void build_multi_assignment(function& function_in, std::vector<statement*>& block);
		void build_if_statements_from_map(function& function_in, std::vector<statement*>& block, block_info* const& previous_block,
			std::unordered_map<statement*, std::uint32_t>& offset_map);
		void build_if_statements(function& function_in, std::vector<statement*>& block, block_info* const& previous_block);
		void clean_up(function& function_in);
		void clean_up_block(function& function_in, std::vector<statement*>& block, std::uint32_t& variable_counter, std::uint32_t& iterator_counter,
			block_info* const& previous_block);
		expression* new_slot(const std::uint8_t& slot);
		expression* new_literal(const std::uint8_t& literal);
		expression* new_signed_literal(const std::uint16_t& signed_literal);
		expression* new_primitive(const std::uint8_t& primitive);
		expression* new_number(const function& function_in, const std::uint16_t& index);
		expression* new_string(const function& function_in, const std::uint16_t& index);
		expression* new_table(const function& function_in, const std::uint16_t& index);
		expression* new_cdata(const function& function_in, const std::uint16_t& index);
		expression* new_xhash(const function& function_in, const std::uint16_t& index);
		void check_special_number(expression* const& expression_in, const bool& is_cdata = false);

		function* chunk_ = nullptr;
		const bytecode& bc_;
		const bool ignore_debug_info_;
		const bool minimize_diffs_;
		bool is_fr2_enabled_ = false;
		std::vector<statement*> statements_{};
		std::vector<function*> functions_{};
		std::vector<expression*> expressions_{};
		std::uint64_t prototype_data_left_ = 0;
	};
}

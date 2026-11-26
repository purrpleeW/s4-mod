#pragma once
#include "common_core.hpp"
#include "lua_decompiler/ast_impl.hpp"
#include "lua_decompiler/bytecode_impl.hpp"

namespace lua_decompiler {
	class lua {
	public:
		lua(const bytecode& bytecode_in, const ast& ast_in, const std::string& file_path, const bool& force_overwrite, const bool& minimize_diffs,
			const bool& unrestricted_ascii);
		~lua();

		static constexpr char UTF8_BOM[] = "\xEF\xBB\xBF";
		static constexpr char NEW_LINE[] = "\r\n";

		void operator()();

		void write_header();
		void write_block(const ast::function& function, const std::vector<ast::statement*>& block);
		void write_expression(const ast::expression& expression, const bool& use_parentheses, const bool& is_table_index = false);
		void write_prefix_expression(const ast::expression& expression, const bool& is_line_start);
		void write_variable(const ast::variable& variable, const bool& is_line_start);
		void write_function_call(const ast::function_call& function_call, const bool& is_line_start);
		void write_assignment(const std::vector<ast::variable>& variables, const std::vector<ast::expression*>& expressions, const std::string& separator,
			const bool& is_line_start);
		void write_expression_list(const std::vector<ast::expression*>& expressions, const ast::expression* const& multres);
		void write_function_definition(const ast::function& function, const bool& is_method);
		void write_number(const double& number);
		void write_string(const std::string& string);
		std::uint8_t get_operator_precedence(const ast::expression& expression);
		void write(const std::string& string);
		void write_indent();
		void create_file();
		void close_file();
		void write_file();

		template <typename... Strings>
		void write(const std::string& string, const Strings&... strings) {
			this->write(string);
			return this->write(strings...);
		}

		const std::string file_path_;
		const bytecode& bc_;
		const ast& ast_;
		const bool force_overwrite_;
		const bool minimize_diffs_;
		const bool unrestricted_ascii_;
		HANDLE file_ = INVALID_HANDLE_VALUE;
		std::string write_buffer_;
		std::uint32_t indent_level_ = 0;
		std::uint64_t prototype_data_left_ = 0;
		std::unordered_map<std::string, std::string> index_map_{};
		std::unordered_map<std::string, std::unordered_map<std::uint64_t, std::string>> xhash_map_{};
	};
}

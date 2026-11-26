#pragma once
#include "common_core.hpp"
#include "lua_decompiler/ast_impl.hpp"

namespace lua_decompiler {
	struct ast::function {
		struct upvalue {
			std::uint8_t slot_ = 0;
			slot_scope** slot_scope_ = nullptr;
			bool local_ = false;
		};

		struct label {
			std::string name_;
			std::uint32_t target_ = INVALID_ID;
			std::vector<std::uint32_t> jump_ids_;
		};

		struct slot_scope_collector {
			struct upvalue_info {
				enum class type {
					t_jump,
					t_upvalues,
					t_upvalue_close
				};

				type type_;
				std::uint32_t id_ = INVALID_ID;
				std::uint32_t target_ = INVALID_ID;
				std::vector<std::uint8_t> upvalues_{};
				std::uint8_t base_slot_ = 0;
			};

			struct upvalue_scope {
				std::uint8_t slot_ = 0;
				std::uint32_t min_scope_begin_ = INVALID_ID;
				std::uint32_t min_scope_end_ = INVALID_ID;
			};

			struct slot_info {
				bool is_parameter_ = false;
				slot_scope** active_slot_scope_ = nullptr;
				std::uint32_t min_scope_begin_ = INVALID_ID;
				std::vector<slot_scope**> slot_scopes_{};
			};

			slot_scope** new_slot_scope();
			std::uint32_t add_upvalue_info(const std::uint32_t& id, const upvalue_info::type& type_in);
			void add_upvalues(const std::uint32_t& id, std::vector<std::uint8_t>& upvalues);
			void add_jump(const std::uint32_t& id, const std::uint32_t& target);
			void add_upvalue_close(const std::uint32_t& id, const std::uint32_t& target, const std::uint8_t& base_slot);
			void add_loop(const std::uint32_t& id, const std::uint32_t& target);
			void add_upvalue_scope(const std::uint8_t& slot, const std::uint32_t& min_scope_begin, const std::uint32_t& min_scope_end);
			void build_upvalue_scopes();
			void begin_scope(const std::uint8_t& slot, const std::uint32_t& id);
			void begin_upvalue_scopes(const std::uint32_t& id);
			void add_to_scope(const std::uint8_t& slot, slot_scope**& slot_scope_in, const std::uint32_t& id);
			void close_scope(const std::uint8_t& slot, slot_scope**& slot_scope_in, const std::uint32_t& id);
			void extend_scope(const std::uint8_t& slot, const std::uint32_t& id);
			void extend_scopes(const std::uint32_t& id);
			void merge_scopes(const std::uint32_t& id);
			bool assert_scopes_closed();
			void remove_scope(const std::uint8_t& slot, slot_scope** const& slot_scope_in);

			std::vector<upvalue_info> upvalue_infos_{};
			std::vector<upvalue_scope> upvalue_scopes_{};
			std::vector<slot_info> slot_infos_{};
			std::vector<slot_scope*> slot_scopes_{};
			std::uint32_t previous_id_ = INVALID_ID;
		};

		function(const bytecode::prototype& prototype, const std::uint32_t& level, const bool& ignore_debug_info);
		~function();
		const bytecode::constant& get_constant(const std::uint16_t& index) const;
		const bytecode::number_constant& get_number_constant(const std::uint16_t& index) const;
		void add_jump(const std::uint32_t& id, const std::uint32_t& target);
		void remove_jump(const std::uint32_t& id, const std::uint32_t& target);
		std::uint32_t get_label_from_id(const std::uint32_t& id);
		bool is_valid_label(const std::uint32_t& label);
		std::uint32_t get_scope_begin_from_label(const std::uint32_t& label, const std::uint32_t& scope_end);
		std::uint32_t get_scope_end_from_label(const std::uint32_t& label);
		bool is_valid_block_range(const std::uint32_t& block_begin, const std::uint32_t& block_end, const bool& ignore_front_label);

		const bytecode::prototype& prototype_;
		const bool is_variadic_;
		const bool has_debug_info_;
		const std::uint32_t level_;
		std::uint32_t id_ = 0;
		bool assignment_slot_is_upvalue_ = false;
		std::vector<local> locals_{};
		std::vector<upvalue> upvalues_{};
		std::vector<label> labels_{};
		std::vector<std::string> parameter_names_{};
		std::vector<statement*> block_{};
		std::vector<function*> child_functions_{};
		std::vector<const std::string*> used_globals_{};
		slot_scope_collector slot_scope_collector_;
	};
}

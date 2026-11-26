#pragma once
#include "common_core.hpp"

namespace lua_decompiler {
	class bytecode {
	public:
		enum class opcode;

		struct constant;
		struct instruction;
		struct number_constant;
		struct prototype;
		struct table_constant;
		struct table_node;
		struct variable_info;

		static constexpr std::uint16_t op_jmp_bias = 0x8000;

		static constexpr std::uint8_t header[] = {
			VAL_AS(std::uint8_t, '\x1B'), VAL_AS(std::uint8_t, 'L'), VAL_AS(std::uint8_t, 'J')
		};

		static constexpr std::uint8_t min_proto_size = 11;
		static constexpr std::uint8_t min_file_size = min_proto_size + 7;
		static constexpr std::uint8_t version_01 = 0x01;
		static constexpr std::uint8_t version_02 = 0x02;
		static constexpr std::uint8_t version_82_t9 = 0x82;
		static constexpr std::uint8_t version_83_iw9 = 0x83;
		static constexpr std::uint8_t version_84_t10 = 0x84;
		static constexpr std::uint8_t f_be = 0x01;
		static constexpr std::uint8_t f_strip = 0x02;
		static constexpr std::uint8_t f_ffi = 0x04;
		static constexpr std::uint8_t f_fr2 = 0x08;
		static constexpr std::uint8_t f_strip_var = 0x10;
		static constexpr std::uint8_t proto_child = 0x01;
		static constexpr std::uint8_t proto_vararg = 0x02;
		static constexpr std::uint8_t proto_ffi = 0x04;
		static constexpr std::uint16_t uv_immutable = 0x4000;
		static constexpr std::uint16_t uv_local = 0x8000;

		static opcode get_op_type(const std::uint8_t& byte, const std::uint8_t& version);
		static bool is_op_abc_format(const opcode& instruction);

		bytecode(const std::string& file_path, const std::string& file_data = {});
		~bytecode();

		void operator()();

		const std::string file_path_;

		struct {
			std::uint8_t version_ = 0;
			std::uint8_t flags_ = 0;
			std::string chunk_name_;
		} header_;

		const prototype* main_ = nullptr;
		std::uint64_t prototypes_total_size_ = 0;
	private:
		void read_header();
		void read_prototypes();
		void open_file();
		void close_file();
		void read_file(const std::uint32_t& byte_count);
		std::uint32_t read_uleb128();
		bool buffer_next_block();

		HANDLE file_ = INVALID_HANDLE_VALUE;
		bool using_raw_data_ = false;
		std::size_t file_size_ = 0;
		std::size_t bytes_unread_ = 0;
		std::size_t raw_data_offset_ = 0;
		std::string raw_data_buffer_{};
		std::vector<std::uint8_t> file_buffer_{};
		std::vector<prototype*> prototypes_{};
	};
}

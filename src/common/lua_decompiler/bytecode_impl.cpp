#include "common_core.hpp"
#include "lua_decompiler/bytecode_impl.hpp"

#include "lua_decompiler/ld_common.hpp"
#include "lua_decompiler/bytecode/opcode.hpp"
#include "lua_decompiler/bytecode/prototype.hpp"
#include "utils/string.hpp"

namespace lua_decompiler {
	namespace {
		using opcode_list = std::vector<bytecode::opcode>;

		const std::unordered_map<std::uint8_t, opcode_list> skipped_opcodes{
			{
				bytecode::version_01, {
					bytecode::opcode::op_istype,
					bytecode::opcode::op_isnum,
					bytecode::opcode::op_khash,
					bytecode::opcode::op_v84_unk1,
					bytecode::opcode::op_v84_unk2,
					bytecode::opcode::op_tgetr,
					bytecode::opcode::op_tsetr
				}
			},
			{
				bytecode::version_02, {
					bytecode::opcode::op_khash,
					bytecode::opcode::op_v84_unk1,
					bytecode::opcode::op_v84_unk2
				}
			},
			{
				bytecode::version_82_t9, {
					bytecode::opcode::op_v84_unk1,
					bytecode::opcode::op_v84_unk2
				}
			},
			{
				bytecode::version_83_iw9, {
					bytecode::opcode::op_v84_unk1,
					bytecode::opcode::op_v84_unk2
				}
			}
		};

		std::unordered_map<std::uint8_t, opcode_list> build_opcode_mappings() {
			std::unordered_map<std::uint8_t, opcode_list> mappings;
			for (const auto& [version, skipped] : skipped_opcodes) {
				std::unordered_set<std::uint32_t> skipped_set;
				for (const auto op : skipped) {
					skipped_set.insert(ENUM_UNDER(op));
				}
				auto& map = mappings[version];
				for (std::uint32_t x = 0; x < ENUM_UNDER(bytecode::opcode::op_count); ++x) {
					if (skipped_set.contains(x)) {
						continue;
					}
					map.push_back(VAL_AS(bytecode::opcode, x));
				}
			}
			return mappings;
		}
	}

	bytecode::opcode bytecode::get_op_type(const std::uint8_t& byte, const std::uint8_t& version) {
		static const auto opcode_mappings = build_opcode_mappings();
		if (auto it = opcode_mappings.find(version); it != opcode_mappings.end() && byte < it->second.size()) {
			return it->second[byte];
		}
		return VAL_AS(opcode, byte);
	}

	bool bytecode::is_op_abc_format(const bytecode::opcode& instruction) {
		switch (instruction) {
		case opcode::op_addvn:
		case opcode::op_subvn:
		case opcode::op_mulvn:
		case opcode::op_divvn:
		case opcode::op_modvn:
		case opcode::op_addnv:
		case opcode::op_subnv:
		case opcode::op_mulnv:
		case opcode::op_divnv:
		case opcode::op_modnv:
		case opcode::op_addvv:
		case opcode::op_subvv:
		case opcode::op_mulvv:
		case opcode::op_divvv:
		case opcode::op_modvv:
		case opcode::op_pow:
		case opcode::op_cat:
		case opcode::op_tgetv:
		case opcode::op_tgets:
		case opcode::op_tgetb:
		case opcode::op_tgetr:
		case opcode::op_tsetv:
		case opcode::op_tsets:
		case opcode::op_tsetb:
		case opcode::op_tsetr:
		case opcode::op_callm:
		case opcode::op_call:
		case opcode::op_iterc:
		case opcode::op_itern:
		case opcode::op_varg:
			return true;
		default:
			return false;
		}
	}

	bytecode::bytecode(const std::string& file_path, const std::string& file_data)
		: file_path_(file_path)
	{
		if (file_data.empty()) {
			return;
		}
		this->using_raw_data_ = true;
		this->raw_data_buffer_ = file_data;
	}

	bytecode::~bytecode() {
		this->close_file();
		for (auto x = this->prototypes_.size(); x--;) {
			delete this->prototypes_[x];
		}
	}

	void bytecode::operator()() {
		// print_progress_bar();
		this->open_file();
		this->read_header();
		this->prototypes_total_size_ = this->bytes_unread_ - 1;
		this->read_prototypes();
		this->close_file();
		this->file_buffer_.clear();
		this->file_buffer_.shrink_to_fit();
		// erase_progress_bar();
	}

	void bytecode::read_header() {
		this->read_file(5);
		ld_assert(memcmp(this->file_buffer_.data(), header, sizeof(header)) == 0, std::format("Invalid header: Expected {}, got {}.",
			utils::string::dump_hex(std::string(PTR_AS(const char*, header), sizeof(header))),
			utils::string::dump_hex(std::string(PTR_AS(const char*, this->file_buffer_.data()), sizeof(header)))), this->file_path_);
		this->header_.version_ = this->file_buffer_[3];
		// assert(header.version == BC_VERSION_1 || header.version == BC_VERSION_2, "Invalid bytecode version (" + byte_to_string(fileBuffer[3]) + ")", filePath, DEBUG_INFO);
		this->header_.flags_ = this->file_buffer_[4];
		// assert(!(header.flags & ~(BC_F_BE | BC_F_STRIP | BC_F_FFI | (header.version == BC_VERSION_2 ? BC_F_FR2 : 0))), "Invalid flags (" + byte_to_string(header.flags) + ")", filePath, DEBUG_INFO);
		// assert(!(header.flags & BC_F_BE), "Big endian support not implemented", filePath, DEBUG_INFO); //TODO
		if (this->header_.flags_ & f_strip || (this->header_.version_ >= version_82_t9 && (this->header_.flags_ & f_strip_var))) {
			return;
		}
		this->read_file(this->read_uleb128());
		this->header_.chunk_name_.resize(this->file_buffer_.size());
		this->header_.chunk_name_.replace(this->header_.chunk_name_.begin(), this->header_.chunk_name_.end(),
			this->file_buffer_.begin(), this->file_buffer_.end());
	}

	void bytecode::read_prototypes() {
		std::vector<prototype*> unlinked_prototypes{};
		while (this->buffer_next_block()) {
			// assert(fileBuffer.size() >= MIN_PROTO_SIZE, "Prototype is too short", filePath, DEBUG_INFO);
			this->prototypes_.emplace_back(new prototype(*this));
			(*this->prototypes_.back())(unlinked_prototypes);
			// print_progress_bar(prototypesTotalSize - bytesUnread - 1, prototypesTotalSize);
		}

		ld_assert(unlinked_prototypes.size() == 1, "Failed to link main prototype", this->file_path_);
		this->main_ = unlinked_prototypes.back();
		ld_assert((this->main_->header_.flags_ & proto_vararg)
			&& !this->main_->header_.parameters_
			&& !this->main_->upvalues_.size(),
			"Main prototype has invalid header", this->file_path_);
		this->prototypes_.shrink_to_fit();
	}

	void bytecode::open_file() {
		if (this->using_raw_data_) {
			this->bytes_unread_ = this->file_size_ = this->raw_data_buffer_.size();
			return;
		}
		this->file_ = CreateFileA(this->file_path_.c_str(), GENERIC_READ, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
		ld_assert(this->file_ != INVALID_HANDLE_VALUE, "Unable to open file", this->file_path_);
		this->file_size_ |= (std::uint64_t)GetFileSize(this->file_, (DWORD*)&this->file_size_) << 32;
		this->file_size_ = (this->file_size_ >> 32) | (this->file_size_ << 32);
		ld_assert(this->file_size_ >= min_file_size, "File is too small or empty", this->file_path_);
		this->bytes_unread_ = this->file_size_;
	}

	void bytecode::close_file() {
		if (this->using_raw_data_ || this->file_ == INVALID_HANDLE_VALUE) {
			return;
		}
		CloseHandle(this->file_);
		this->file_ = INVALID_HANDLE_VALUE;
	}

	void bytecode::read_file(const std::uint32_t& byte_count) {
		ld_assert(this->bytes_unread_ >= byte_count, "Read would exceed end of file", this->file_path_);
		this->file_buffer_.resize(byte_count);
		if (this->using_raw_data_) {
			for (auto x = this->raw_data_offset_; x < this->raw_data_offset_ + byte_count; x++) {
				this->file_buffer_[x - this->raw_data_offset_] = this->raw_data_buffer_[x];
			}
			this->raw_data_offset_ += byte_count;
		}
		else {
			DWORD bytes_read = 0;
			ld_assert(ReadFile(this->file_, this->file_buffer_.data(), byte_count, &bytes_read, NULL) && !(byte_count - bytes_read),
				"Failed to read file", this->file_path_);
		}
		this->bytes_unread_ -= byte_count;
	}

	std::uint32_t bytecode::read_uleb128() {
		this->read_file(1);
		std::uint32_t uleb128 = this->file_buffer_[0];
		if (uleb128 >= 0x80) {
			uleb128 &= 0x7F;
			std::uint8_t bit_shift = 0;
			do {
				bit_shift += 7;
				this->read_file(1);
				uleb128 |= (this->file_buffer_[0] & 0x7F) << bit_shift;
			} while (this->file_buffer_[0] >= 0x80);
		}
		return uleb128;
	}

	bool bytecode::buffer_next_block() {
		const std::uint32_t byte_count = this->read_uleb128();
		if (!byte_count) {
			ld_assert(!this->bytes_unread_, "Read unexpectedly reached end of file", this->file_path_);
			return false;
		}
		this->read_file(byte_count);
		return true;
	}
}

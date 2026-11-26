#include "common_core.hpp"
#include "lua_decompiler/bytecode/prototype.hpp"

#include "lua_decompiler/ld_common.hpp"

namespace lua_decompiler {
	bytecode::prototype::prototype(const bytecode& bc)
		: bc_(bc)
	{}

	void bytecode::prototype::operator()(std::vector<prototype*>& unlinked_prototypes) {
		this->read_header();
		this->read_instructions();
		this->read_upvalues();
		this->read_constants(unlinked_prototypes);
		this->read_number_constants();
		this->read_debug_info();
		ld_assert(this->prototype_size_ == this->bc_.file_buffer_.size(), "Prototype has unread bytes left", this->bc_.file_path_);
		unlinked_prototypes.emplace_back(this);
	}

	void bytecode::prototype::read_header() {
		this->header_.flags_ = this->get_next_byte();
		// assert(!(header.flags & ~(BC_PROTO_CHILD | BC_PROTO_VARARG | BC_PROTO_FFI)), "Prototype has invalid flags (" + byte_to_string(header.flags) + ")", bytecode.filePath, DEBUG_INFO);
		this->header_.parameters_ = this->get_next_byte();
		this->header_.frame_size_ = this->get_next_byte();
		this->upvalues_.resize(this->get_next_byte());
		this->constants_.resize(this->get_uleb128());
		this->number_constants_.resize(this->get_uleb128());
		this->instructions_.resize(this->get_uleb128());
		ld_assert(this->instructions_.size(), "Prototype has no instructions", this->bc_.file_path_);
		if (this->bc_.header_.flags_ & f_strip || !this->get_uleb128()) {
			return;
		}
		this->header_.has_debug_info_ = this->bc_.header_.version_ <= version_02 || !(this->bc_.header_.flags_ & f_strip_var);
		this->header_.has_line_info_ = true;
		this->header_.first_line_ = this->get_uleb128();
		this->header_.line_count_ = this->get_uleb128();
	}

	void bytecode::prototype::read_instructions() {
		for (auto x = 0; x < this->instructions_.size(); x++) {
			this->instructions_[x].type_ = get_op_type(this->get_next_byte(), this->bc_.header_.version_);
			ld_assert(ENUM_UNDER(this->instructions_[x].type_) < ENUM_UNDER(opcode::op_count), std::format("Prototype has invalid instruction (0x{:X})",
				ENUM_UNDER(this->instructions_[x].type_)), this->bc_.file_path_);

			switch (this->instructions_[x].type_) {
			case opcode::op_istype:
			case opcode::op_isnum:
			case opcode::op_v84_unk1:
			case opcode::op_v84_unk2:
			case opcode::op_tgetr:
			case opcode::op_tsetr:
			case opcode::op_jfori:
			case opcode::op_iforl:
			case opcode::op_jforl:
			case opcode::op_iiterl:
			case opcode::op_jiterl:
			case opcode::op_iloop:
			case opcode::op_jloop:
			case opcode::op_funcf:
			case opcode::op_ifuncf:
			case opcode::op_jfuncf:
			case opcode::op_funcv:
			case opcode::op_ifuncv:
			case opcode::op_jfuncv:
			case opcode::op_funcc:
			case opcode::op_funccw:
				ld_assert(false, std::format("Prototype has unsupported instruction (0x{:X})", ENUM_UNDER(this->instructions_[x].type_)), this->bc_.file_path_);
			default:
				break;
			}

			this->instructions_[x].a_ = this->get_next_byte();
			if (is_op_abc_format(this->instructions_[x].type_)) {
				this->instructions_[x].c_ = this->get_next_byte();
				this->instructions_[x].b_ = this->get_next_byte();
			}
			else {
				this->instructions_[x].d_ = this->get_next_byte();
				this->instructions_[x].d_ |= VAL_AS(std::uint16_t, this->get_next_byte()) << 8;
			}
		}
	}

	void bytecode::prototype::read_upvalues() {
		for (auto x = 0; x < this->upvalues_.size(); x++) {
			this->upvalues_[x] = this->get_next_byte();
			this->upvalues_[x] |= VAL_AS(std::uint16_t, this->get_next_byte()) << 8;
		}
	}

	void bytecode::prototype::read_constants(std::vector<prototype*>& unlinked_prototypes) {
		for (auto x = 0; x < this->constants_.size(); x++) {
			auto type_val = this->get_uleb128();
			if (this->bc_.header_.version_ < version_82_t9 && type_val >= ENUM_UNDER(constant::type::t_xhash)) {
				type_val += 1;
			}

			auto type = VAL_AS(constant::type, type_val);
			switch (type) {
			case constant::type::t_child:
				this->constants_[x].type_ = constant::type::t_child;
				ld_assert(unlinked_prototypes.size(), "Failed to link child prototype", this->bc_.file_path_);
				this->constants_[x].prototype_ = unlinked_prototypes.back();
				unlinked_prototypes.pop_back();
				continue;
			case constant::type::t_tab:
				this->constants_[x].type_ = constant::type::t_tab;
				this->constants_[x].array_.resize(this->get_uleb128());
				this->constants_[x].table_.resize(this->get_uleb128());
				for (auto y = 0; y < this->constants_[x].array_.size(); y++) {
					this->constants_[x].array_[y] = this->get_table_constant();
				}
				for (auto y = 0; y < this->constants_[x].table_.size(); y++) {
					this->constants_[x].table_[y].key_ = this->get_table_constant();
					this->constants_[x].table_[y].value_ = this->get_table_constant();
				}
				continue;
			case constant::type::t_complex:
				ld_assert(!this->get_uleb128() && !this->get_uleb128(), "Prototype has invalid cdata constant", this->bc_.file_path_);
			case constant::type::t_i64:
			case constant::type::t_u64:
				this->constants_[x].type_ = type;
				this->constants_[x].cdata_ = this->get_uleb128();
				this->constants_[x].cdata_ |= VAL_AS(std::uint64_t, this->get_uleb128()) << 0x20;
				continue;
			case constant::type::t_xhash:
				this->constants_[x].type_ = type;
				this->constants_[x].xhash_ = VAL_AS(std::uint64_t, get_uleb128()) << 0x20;
				this->constants_[x].xhash_ |= this->get_uleb128();
				if (this->bc_.header_.version_ >= version_83_iw9) {
					this->constants_[x].xhash_type_ = this->get_next_byte();
				}
				continue;
			default:
				this->constants_[x].type_ = constant::type::t_str;
				this->constants_[x].string_.resize(ENUM_UNDER(type) - ENUM_UNDER(constant::type::t_str));
				for (auto y = 0; y < this->constants_[x].string_.size(); y++) {
					this->constants_[x].string_[y] = get_next_byte();
				}
				continue;
			}
		}
	}

	void bytecode::prototype::read_number_constants() {
		for (auto x = 0; x < this->number_constants_.size(); x++) {
			if (this->bc_.file_buffer_[this->prototype_size_] & 0x01) {
				this->number_constants_[x].type_ = number_constant::type::t_num;
				this->number_constants_[x].number_ = this->get_uleb128_33();
				this->number_constants_[x].number_ |= VAL_AS(std::uint64_t, this->get_uleb128()) << 32;
			}
			else {
				this->number_constants_[x].type_ = number_constant::type::t_int;
				this->number_constants_[x].integer_ = this->get_uleb128_33();
			}
		}
	}

	void bytecode::prototype::read_debug_info() {
		if (!this->header_.has_line_info_) {
			return;
		}

		this->line_map_.resize(this->instructions_.size());
		if (this->header_.line_count_ < 0x100) {
			for (auto x = 0; x < this->line_map_.size(); x++) {
				this->line_map_[x] = this->get_next_byte();
			}
		}
		else if (this->header_.line_count_ < 0x10000) {
			for (auto x = 0; x < this->line_map_.size(); x++) {
				this->line_map_[x] = this->get_next_byte();
				this->line_map_[x] |= VAL_AS(std::uint16_t, this->get_next_byte()) << 0x08;
			}
		}
		else {
			for (auto x = 0; x < this->line_map_.size(); x++) {
				this->line_map_[x] = this->get_next_byte();
				this->line_map_[x] |= VAL_AS(std::uint32_t, this->get_next_byte()) << 0x08;
				this->line_map_[x] |= VAL_AS(std::uint32_t, this->get_next_byte()) << 0x10;
				this->line_map_[x] |= VAL_AS(std::uint32_t, this->get_next_byte()) << 0x18;
			}
		}

		if (!this->header_.has_debug_info_) {
			return;
		}

		this->upvalue_names_.resize(this->upvalues_.size());
		for (auto x = 0; x < this->upvalue_names_.size(); x++) {
			this->upvalue_names_[x] = this->get_string();
		}

		std::uint32_t scope_offset = 0, parameter_count = 0;
		for (std::uint8_t byte = this->get_next_byte(); byte; byte = this->get_next_byte()) {
			this->variable_infos_.emplace_back();
			if (byte >= ENUM_UNDER(variable_info::type::t_str)) {
				this->variable_infos_.back().type_ = variable_info::type::t_str;
				this->variable_infos_.back().name_ += byte;
				this->variable_infos_.back().name_ += this->get_string();
			}
			else {
				this->variable_infos_.back().type_ = VAL_AS(variable_info::type, byte);
			}

			scope_offset += this->get_uleb128();
			ld_assert(scope_offset != 1, "Prototype variable has invalid scope", this->bc_.file_path_);

			if (!scope_offset) {
				this->variable_infos_.back().is_parameter_ = true;
				parameter_count++;
				this->variable_infos_.back().scope_end_ = this->get_uleb128() - 2;
			}
			else {
				this->variable_infos_.back().scope_begin_ = scope_offset - 2;
				this->variable_infos_.back().scope_end_ = this->variable_infos_.back().scope_begin_ + this->get_uleb128();
			}
		}

		ld_assert(VAL_AS(std::uint8_t, parameter_count) == this->header_.parameters_, "Prototype parameter count does not match with debug info", this->bc_.file_path_);
		this->variable_infos_.shrink_to_fit();
	}

	std::uint8_t bytecode::prototype::get_next_byte() {
		ld_assert(this->prototype_size_ < this->bc_.file_buffer_.size(), "Prototype read would exceed end of buffer", this->bc_.file_path_);
		return this->bc_.file_buffer_[this->prototype_size_++];
	}

	std::uint32_t bytecode::prototype::get_uleb128() {
		std::uint32_t uleb128 = this->get_next_byte();
		if (uleb128 >= 0x80) {
			uleb128 &= 0x7F;
			std::uint8_t byte, bit_shift = 0;
			do {
				bit_shift += 7;
				byte = this->get_next_byte();
				uleb128 |= VAL_AS(std::uint32_t, byte & 0x7F) << bit_shift;
			} while (byte >= 0x80);
		}
		return uleb128;
	}

	std::uint32_t bytecode::prototype::get_uleb128_33() {
		std::uint32_t uleb128_33 = this->get_next_byte() >> 1;
		if (uleb128_33 >= 0x40) {
			uleb128_33 &= 0x3F;
			std::uint8_t byte;
			std::int8_t bit_shift = -1;
			do {
				bit_shift += 7;
				byte = this->get_next_byte();
				uleb128_33 |= VAL_AS(std::uint32_t, byte & 0x7F) << bit_shift;
			} while (byte >= 0x80);
		}
		return uleb128_33;
	}

	std::string bytecode::prototype::get_string() {
		std::string string{};
		for (std::uint8_t byte = this->get_next_byte(); byte; byte = this->get_next_byte()) {
			string += VAL_AS(char, byte);
		}
		return string;
	}

	bytecode::table_constant bytecode::prototype::get_table_constant() {
		table_constant table_const;
		auto type_val = this->get_uleb128();
		if (this->bc_.header_.version_ < version_82_t9 && type_val >= ENUM_UNDER(table_constant::type::t_xhash)) {
			type_val += 1;
		}

		auto type = VAL_AS(table_constant::type, type_val);
		switch (type) {
		case table_constant::type::t_nil:
		case table_constant::type::t_false:
		case table_constant::type::t_true:
			table_const.type_ = type;
			break;
		case table_constant::type::t_int:
			table_const.type_ = table_constant::type::t_int;
			table_const.integer_ = this->get_uleb128();
			break;
		case table_constant::type::t_num:
			table_const.type_ = table_constant::type::t_num;
			table_const.number_ = this->get_uleb128();
			table_const.number_ |= VAL_AS(std::uint64_t, this->get_uleb128()) << 0x20;
			break;
		case table_constant::type::t_xhash:
			table_const.type_ = table_constant::type::t_xhash;
			table_const.xhash_ = VAL_AS(std::uint64_t, this->get_uleb128()) << 0x20;
			table_const.xhash_ |= this->get_uleb128();
			if (this->bc_.header_.version_ >= version_83_iw9) {
				table_const.xhash_type_ = this->get_next_byte();
			}
			break;
		default:
			table_const.type_ = table_constant::type::t_str;
			table_const.string_.resize(ENUM_UNDER(type) - ENUM_UNDER(table_constant::type::t_str));
			for (auto x = 0; x < table_const.string_.size(); x++) {
				table_const.string_[x] = this->get_next_byte();
			}
			break;
		}

		return table_const;
	}
}

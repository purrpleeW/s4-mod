#pragma once

#include "common_core.hpp"
#include <logger/logger.hpp>

#include <utils/hook.hpp>

namespace arxan::detail {
	enum class integrity_check_type {
		none		= 0,
		flag_intact	= 1 << 0,
		flag_split	= 1 << 1,
		flag_basic	= 1 << 2,
		flag_big	= 1 << 3,

		intact_basic	= flag_intact | flag_basic,
		intact_big		= flag_intact | flag_big,
		split_basic		= flag_split | flag_basic,
		split_big		= flag_split | flag_big
	};
	DECLARE_BITWISE_OPS(integrity_check_type);

	enum class code_healer_type {
		none		= 0,
		flag_eax	= 1 << 0,
		flag_al		= 1 << 1,
		flag_basic	= 1 << 2,
		flag_lea	= 1 << 3,
		flag_jmp	= 1 << 4,

		eax_basic	= flag_eax | flag_basic,
		eax_lea		= flag_eax | flag_lea | flag_jmp,
		eax_jmp		= flag_eax | flag_jmp,
		al_basic	= flag_al | flag_basic,
		al_lea		= flag_al | flag_lea | flag_jmp,
		al_jmp		= flag_al | flag_jmp
	};
	DECLARE_BITWISE_OPS(code_healer_type);

	struct enum_cls_hash_functor {
		template <typename T>
		std::size_t operator()(T t) const {
			return VAL_AS(std::size_t, t);
		}
	};

	struct cached_code_block {
		void store() {
			this->stored_data_.clear();
			this->stored_data_.reserve(this->data_size_);
			for (auto x = 0; x < this->data_size_; x++) {
				this->stored_data_.push_back(this->location_.pointer_bytes_[x]);
			}
		}

		bool fix_data() {
			if (std::memcmp(this->location_.pointer_, this->stored_data_.data(), this->stored_data_.size()) == 0) {
				return false;
			}

			utils::hook::copy(this->location_.address_, this->stored_data_.data(), this->stored_data_.size(), true);
			return true;
		}

		union _location {
			_location(void* p) {
				this->pointer_ = p;
			};

			std::uintptr_t address_;
			std::uint8_t* pointer_bytes_;
			void* pointer_;
		} location_;

		const std::size_t data_size_;
		std::vector<std::uint8_t> stored_data_{};
	};

	inline std::vector<void*> potential_integrity_checks{};
	inline std::unordered_map<integrity_check_type, std::vector<void*>, enum_cls_hash_functor> integrity_check_map{};
	inline std::unordered_map<code_healer_type, std::vector<void*>, enum_cls_hash_functor> code_healer_map{};

	inline std::vector<cached_code_block> code_healers_instruction_cache{};
	inline std::vector<cached_code_block> code_healers_original_instruction_cache{};
	inline std::vector<cached_code_block> integrity_checks_instruction_cache{};
	inline std::vector<cached_code_block> integrity_checks_original_instruction_cache{};
}
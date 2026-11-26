#pragma once
#include "common.hpp"
#include <memory/memory.hpp>
#include <utils/flags.hpp>
#include <utils/hook.hpp>

namespace arxan {
	enum checksum_location_type {
		Intact, IntactBig, Split
	};

	struct checksum_stack_data {
		void store() {
			this->stored_data_.clear();
			this->stored_data_.reserve(this->data_size_);
			for (auto x = 0; x < this->data_size_; x++) {
				this->stored_data_.push_back(this->location_.pointer_bytes_[x]);
			}
		}

		union _location {
			_location(void* p) { this->pointer_ = p; };

			std::uintptr_t address_;
			std::uint8_t* pointer_bytes_;
			void* pointer_;
		} location_;

		const checksum_location_type type_;
		const std::size_t data_size_;
		std::vector<std::uint8_t> stored_data_{};
	};

	struct checksum_healing_location {
		std::vector<void*> pattern;
		const std::size_t length;
		std::function<void(asmjit::x86::Assembler& a, asmjit::Label& label, std::uintptr_t jmp_location)> handler;
	};

	inline std::vector<checksum_stack_data> stored_checksum_locations{};
	inline bool should_notify_on_exception = true;

	namespace asm_impl {
		void push_ad64(asmjit::x86::Assembler* a);
		void pop_ad64(asmjit::x86::Assembler* a);
		void pop_ad64_no_rax(asmjit::x86::Assembler* a);
		void push_ad64_min(asmjit::x86::Assembler* a);
		void pop_ad64_min(asmjit::x86::Assembler* a);
	}
}

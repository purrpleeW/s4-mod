#pragma once

#include "common_core.hpp"
#include <logger/logger.hpp>

#include <utils/bit_flags.hpp>
#include <utils/hook.hpp>
#include <memory/memory.hpp>

#include "arxan_detail.hpp"

namespace arxan::detail::integrity_checks {
	using namespace asmjit::x86;

	struct patched_code_section {
		void* stub_location;
		std::size_t stub_size;
		std::size_t patch_area;

		patched_code_section(std::pair<void*, std::size_t> stub, std::size_t patch_area)
			: stub_location(stub.first)
			, stub_size(stub.second)
			, patch_area(patch_area)
		{}
	};

	constexpr std::uint32_t reverse_bytes(const std::uint32_t x) noexcept {
		return std::byteswap(x);
	}

	inline void assign_integrity_checks(const std::vector<void*>& potential_checks) {
		PROTECT_VOID(categorized);
		static utils::nt::library game{};
		for (auto& potential_location : potential_checks) {
			auto factor_addr = PTR_AS(std::uintptr_t, potential_location) + 3;
			if (!game.is_address_in_accessible_range(factor_addr)) {
				continue;
			}
			if (memory::matches(factor_addr, "83 45 ? FF")) {
				integrity_check_map[integrity_check_type::intact_basic].push_back(potential_location);
			}
			else if (memory::matches(factor_addr, "83 85")) {
				integrity_check_map[integrity_check_type::intact_big].push_back(potential_location);
			}
			else if (memory::matches(factor_addr, "E9")) {
				integrity_check_map[integrity_check_type::split_basic].push_back(potential_location);
			}
			else if (memory::matches(factor_addr, "? ? ? 24 F8")) {
				integrity_check_map[integrity_check_type::split_big].push_back(potential_location);
			}
			else {
				// disregarded - either an invalid location or an undiscovered type.
			}
		}
	}

	inline bool validate_split_jump_target(void* target) {
		static utils::nt::library game{};
		return target && game.is_address_in_accessible_range(PTR_AS(std::uintptr_t, target))
			&& (memory::matches(target, "83 45 ? FF") || memory::matches(target, "83 85"));
	}

	inline std::uintptr_t get_rbp_offset(void* address, std::size_t offset) {
		auto bytes = PTR_AS(std::uint8_t*, PTR_AS(std::uintptr_t, address) + offset);
		if (bytes[0] == 0x45) {
			return DEREF_PTR_AS(char, &bytes[1]);
		}
		if (bytes[0] == 0x85) {
			return DEREF_PTR_AS(short, &bytes[1]);
		}
		return 0;
	}

	inline std::uint32_t* obtain_checksum_ptr(std::uint64_t* ptr_stack, std::uint32_t expected_value, int range = 80) {
		// 0x120 is a good starting point to decrement downwards to find the calculated checksum on the stack
		std::uint32_t* ptr = PTR_AS(std::uint32_t*, PTR_AS(std::uintptr_t, ptr_stack) + 0x120);
		for (auto x = 0; x < range; x++) {
			if (*ptr == expected_value) {
				break;
			}
			ptr--;
		}
		return ptr;
	}

	inline void assign_all_stack_checksums(std::uint64_t ptr_offset, std::uint32_t* original_ch, std::uint32_t* calculated_ch, std::uint32_t* reversed_ch) {
		if (original_ch == nullptr) {
			return;
		}

		for (auto x = 0; x <= ptr_offset; x++) {
			*calculated_ch = *original_ch;
			*reversed_ch = reverse_bytes(*original_ch);

			original_ch--; calculated_ch--; reversed_ch--;
		}
	}

	inline int fix_checksum(std::uint64_t rbp_offset, std::uint64_t ptr_offset, std::uint64_t* ptr_stack, std::uint32_t padding,
		std::uint32_t calculated_checksum)
	{
		static utils::nt::library game{};

		auto text_ptr = PTR_AS(std::uintptr_t*, PTR_AS(std::uintptr_t, ptr_stack) + rbp_offset + (rbp_offset % 0x8));
		auto calculated_checksum_ptr = obtain_checksum_ptr(ptr_stack, calculated_checksum);
		auto reversed_checksum_ptr = obtain_checksum_ptr(ptr_stack, reverse_bytes(calculated_checksum));
		auto is_small_check = ptr_offset == 0 && rbp_offset < 0x90;

		for (auto& check : integrity_checks_instruction_cache) {
			check.fix_data();
		}

		bool double_text_checksum = false;
		std::uintptr_t* previous_result_ptr = nullptr;
		if (is_small_check) {
			auto pointer_counter = 0;
			auto tmp_text_ptr = text_ptr;
			for (auto x = 0; x < 20; x++, tmp_text_ptr--) {
				std::uintptr_t deref_ptr = *tmp_text_ptr;
				if (!game.is_address_in_range(deref_ptr)) {
					continue;
				}

				pointer_counter++;
				std::uintptr_t deref_result = DEREF_PTR_AS(std::uintptr_t, deref_ptr);
				if (deref_result == UINTPTR_MAX) {
					double_text_checksum = pointer_counter > 2;
					break;
				}
				previous_result_ptr = tmp_text_ptr;
			}
		}

		std::uint32_t original_checksum = 0;
		std::uint32_t* original_checksum_ptr = nullptr;
		auto tmp_text_ptr = text_ptr;
		for (auto x = 0; x < 10; x++, tmp_text_ptr--) {
			auto deref_ptr = *tmp_text_ptr;
			if (!game.is_address_in_range(deref_ptr)) {
				continue;
			}
			if (is_small_check) {
				if (double_text_checksum) {
					original_checksum = *DEREF_PTR_AS(std::uint32_t*, previous_result_ptr);
				}
				else {
					original_checksum = DEREF_PTR_AS(std::uint32_t, deref_ptr);
				}
			}
			else {
				original_checksum_ptr = PTR_AS(std::uint32_t*, deref_ptr + (ptr_offset * sizeof(std::uint32_t)));
				original_checksum = *original_checksum_ptr;
			}
			break;
		}

		*calculated_checksum_ptr = original_checksum;
		*reversed_checksum_ptr = reverse_bytes(original_checksum);
		assign_all_stack_checksums(ptr_offset, original_checksum_ptr, calculated_checksum_ptr, reversed_checksum_ptr);

		return original_checksum;
	}

	inline std::pair<void*, std::size_t> patch_generic_intact_block_integrity_check(void* address) {
		auto rbp_offset = get_rbp_offset(address, 4);
		return utils::hook::assemble([&rbp_offset](utils::hook::assembler& a) {
			a.sub(rsp, 0x32);

			a.pushad64();
			a.mov(qword_ptr(rsp, 0x20), rax);
			a.mov(rdx, rcx);
			a.mov(rcx, rbp_offset);
			a.mov(r8, rbp);
			a.mov(r9, 0);
			a.mov(rax, PTR_AS(void*, fix_checksum));
			a.call(rax);
			a.add(rsp, 0x20);
			a.popad64_no_rax();

			a.add(rsp, 0x32);
			a.mov(ptr(rdx, rcx, 2), eax);
			a.add(dword_ptr(rbp, rbp_offset), -1);
			a.add(rsp, 0x8);

			a.ret();
		});
	}

	inline std::pair<void*, std::size_t> patch_generic_split_block_integrity_check(void* address, void* jump_target) {
		auto rbp_offset = get_rbp_offset(jump_target, 1);
		return utils::hook::assemble([&rbp_offset, jump_target](utils::hook::assembler& a) {
			a.sub(rsp, 0x32);

			a.pushad64();
			a.mov(qword_ptr(rsp, 0x20), rax);
			a.mov(rdx, rcx);
			a.mov(rcx, rbp_offset);
			a.mov(r8, rbp);
			a.mov(r9, 0);
			a.mov(rax, PTR_AS(void*, fix_checksum));
			a.call(rax);
			a.add(rsp, 0x20);
			a.popad64_no_rax();

			a.add(rsp, 0x32);
			a.mov(ptr(rdx, rcx, 2), eax);
			a.add(rsp, 0x8);
			a.mov(r11, jump_target);
			a.add(rsp, 0x8);
			a.push(r11);

			a.ret();
		});
	}

	inline std::optional<patched_code_section> patch_intact_basic_block_integrity_check(void* address) {
		return patched_code_section(patch_generic_intact_block_integrity_check(address), 7);
	}

	inline std::optional<patched_code_section> patch_intact_big_block_integrity_check(void* address) {
		return patched_code_section(patch_generic_intact_block_integrity_check(address), 10);
	}

	inline std::optional<patched_code_section> patch_split_basic_block_integrity_check(void* address) {
		const auto jump_target = utils::hook::extract<void*>(PTR_AS(void*, PTR_AS(std::uintptr_t, address) + 4));
		if (!validate_split_jump_target(jump_target)) {
			return {};
		}
		return patched_code_section(patch_generic_split_block_integrity_check(address, jump_target), 8);
	}

	inline std::optional<patched_code_section> patch_split_big_block_integrity_check(void* address) {
		const auto jump_target = utils::hook::extract_lea<void*>(PTR_AS(void*, PTR_AS(std::uintptr_t, address) + 3));
		if (!validate_split_jump_target(jump_target)) {
			return {};
		}
		return patched_code_section(patch_generic_split_block_integrity_check(address, jump_target), 8);
	}

	inline std::size_t patch_integrity_check_block(integrity_check_type type) {
		static utils::nt::library game{};

		static std::unordered_map<integrity_check_type, std::function<std::optional<patched_code_section>(void*)>> handlers_map = {
			{ integrity_check_type::intact_basic, patch_intact_basic_block_integrity_check },
			{ integrity_check_type::intact_big,   patch_intact_big_block_integrity_check   },
			{ integrity_check_type::split_basic,  patch_split_basic_block_integrity_check  },
			{ integrity_check_type::split_big, 	  patch_split_big_block_integrity_check    },
		};

		std::unordered_map<std::uintptr_t, std::pair<std::uintptr_t, std::size_t>> stub_calls{};
		std::size_t total_stubs_size{};

		std::size_t count = 0;
		for (auto& check : integrity_check_map[type]) {
			auto handler_it = handlers_map.find(type);
			if (handler_it == handlers_map.end()){
				return 0;
			}

			auto result = handler_it->second(check);
			if (!result.has_value()) {
				continue;
			}

			stub_calls[PTR_AS(std::uintptr_t, check)] = { PTR_AS(std::uintptr_t, result->stub_location), result->stub_size };
			total_stubs_size += result->stub_size;

			auto instruction_cache = cached_code_block{
				.location_ = check, .data_size_ = result->patch_area
			};

			instruction_cache.store();
			integrity_checks_original_instruction_cache.push_back(instruction_cache);

			utils::hook::nop(instruction_cache.location_.address_, result->patch_area, true);
			utils::hook::call(instruction_cache.location_.address_, result->stub_location);
			instruction_cache.store();
			integrity_checks_instruction_cache.push_back(instruction_cache);

			count++;
		}

		if (total_stubs_size) {
			auto current = PTR_AS(std::uintptr_t, utils::hook::allocate_somewhere_near(game.get_ptr(), total_stubs_size));
			for (auto& [check, stub_inf] : stub_calls) {
				auto& [stub, stub_size] = stub_inf;
				if (!utils::hook::is_relatively_far(check, stub)) {
					continue;
				}
				utils::hook::copy(current, PTR_AS(void*, stub), stub_size);
				utils::hook::call(check, current);
				current += stub_size;
			}
		}

		return count;
	}
}
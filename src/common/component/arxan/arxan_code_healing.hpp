#pragma once

#include "common_core.hpp"
#include <logger/logger.hpp>

#include <utils/bit_flags.hpp>
#include <utils/hook.hpp>
#include <memory/memory.hpp>

#include "arxan_detail.hpp"

namespace arxan::detail::code_healing {
	using namespace asmjit::x86;

	inline bool overlaps_region(const std::uintptr_t heal_start, const std::uintptr_t heal_end, const std::vector<cached_code_block>& region) {
		for (auto& entry : region) {
			const auto patch_start = entry.location_.address_;
			const auto patch_end = patch_start + entry.data_size_;
			if (heal_start < patch_end && heal_end > patch_start) {
				return true;
			}
		}
		return false;
	}

	inline bool allow_code_healing(void* heal_location, const std::uint32_t patch_length) {
		const auto heal_start = PTR_AS(std::uintptr_t, heal_location);
		const auto heal_end = heal_start + patch_length;
		return !overlaps_region(heal_start, heal_end, code_healers_instruction_cache)
			&& !overlaps_region(heal_start, heal_end, integrity_checks_instruction_cache);
	}

	inline std::optional<std::pair<void*, std::size_t>> generate_code_healer_stub(void* address, utils::bit_flags<code_healer_type> flags) {
		static utils::nt::library game{};

		void* jump_target = nullptr;
		bool jump_target_expected = false;
		if (flags & code_healer_type::flag_lea) {
			jump_target_expected = true;
			jump_target = utils::hook::extract_lea<void*>(address);
		}
		else if (flags & code_healer_type::flag_jmp) {
			jump_target_expected = true;
			jump_target = utils::hook::extract<void*>(PTR_AS(void*, PTR_AS(std::uintptr_t, address) + 3));
		}

		if (jump_target_expected) {
			if (!jump_target || !game.is_address_in_accessible_range(PTR_AS(std::uintptr_t, jump_target))) {
				return {};
			}

			std::uint8_t* expected_memory = nullptr;
			std::size_t expected_memory_size = 0;

			static std::uint8_t instruction_al[] = {
				0x8B, 0x45, 0x20,	// mov eax, dword ptr [rbp+0x20]
				0xFF				// [...]
			};

			static std::uint8_t instruction_eax[] = {
				0x8B, 0x45, 0x20	// mov eax, dword ptr [rbp+0x20]
			};

			if (flags & code_healer_type::flag_al) {
				expected_memory = instruction_al;
				expected_memory_size = sizeof(instruction_al);
			}
			else if (flags & code_healer_type::flag_eax) {
				expected_memory = instruction_eax;
				expected_memory_size = sizeof(instruction_eax);
			}

			if (expected_memory == nullptr || expected_memory_size == 0) {
				return {};
			}

			if (std::memcmp(jump_target, expected_memory, expected_memory_size) != 0) {
				return {};
			}
		}

		return utils::hook::assemble([&](utils::hook::assembler& a) {
			if (flags & code_healer_type::flag_lea) {
				a.mov(rdx, qword_ptr(rbp, 0x10));	// mov rdx, [rbp+10h]
				a.mov(eax, dword_ptr(rax));			// mov eax, [rax]
			}

			a.pushad64();

			const auto skip_update = a.newLabel();

			a.mov(rcx, rdx);	// rdx = destination pointer
			if (flags & code_healer_type::flag_al) {
				a.mov(edx, 1);	// length = 1 byte (size of al)
			}
			else if (flags & code_healer_type::flag_eax) {
				a.mov(edx, 4);	// length = 4 byte (size of al)
			}
			a.call_aligned(allow_code_healing);

			a.test(al, al);
			a.jz(skip_update);

			a.popad64();

			if (flags & code_healer_type::flag_al) {
				a.mov(byte_ptr(rdx), al);			// mov [rdx], al
			}
			else if (flags & code_healer_type::flag_eax) {
				a.mov(dword_ptr(rdx), eax);			// mov [rdx], eax
			}

			if (flags & code_healer_type::flag_jmp) {
				a.add(rsp, 8);						// Remove return address from stack
				a.jmp(jump_target);
			}
			else if (flags & code_healer_type::flag_al) {
				a.add(dword_ptr(rbp, 0x20), -1);	// add dword ptr [rbp+20h], 0FFFFFFFFh
				a.ret();
			}
			else if (flags & code_healer_type::flag_eax) {
				a.mov(eax, dword_ptr(rbp, 0x20));	// mov eax, [rbp+20h]
				a.ret();
			}

			a.bind(skip_update);
			a.popad64();

			if (flags & code_healer_type::flag_jmp) {
				// Remove return address from stack
				a.add(rsp, 8);
				a.jmp(jump_target);
			}
			else if (flags & code_healer_type::flag_al) {
				// Skip overwrite but decrease counter
				a.add(dword_ptr(rbp, 0x20), -1);
				a.ret();
			}
			else if (flags & code_healer_type::flag_eax) {
				// Skip overwrite but reload original eax value
				a.mov(eax, dword_ptr(rbp, 0x20));
				a.ret();
			}
		});
	}

	inline std::size_t patch_code_healer_block(code_healer_type type) {
		static utils::nt::library game{};

		static std::unordered_map<code_healer_type, std::size_t, enum_cls_hash_functor> patch_area_sizes = {
			{ code_healer_type::al_jmp,    7 },
			{ code_healer_type::eax_lea,   6 },
			{ code_healer_type::eax_jmp,   7 },
			{ code_healer_type::al_basic,  6 },
			{ code_healer_type::eax_basic, 5 },
		};

		std::unordered_map<std::uintptr_t, std::pair<std::uintptr_t, std::size_t>> stub_calls{};
		std::size_t total_stubs_size{};

		std::size_t count = 0;
		for (auto& check : code_healer_map[type]) {
			auto result = generate_code_healer_stub(check, utils::bit_flags(type));
			if (!result.has_value()) {
				continue;
			}

			auto size_it = patch_area_sizes.find(type);
			if (size_it == patch_area_sizes.end()) {
				continue;
			}

			auto [stub, stub_size] = *result;
			auto patch_area = size_it->second;

			stub_calls[PTR_AS(std::uintptr_t, check)] = { PTR_AS(std::uintptr_t, stub), stub_size };
			total_stubs_size += stub_size;

			auto instruction_cache = cached_code_block{
				.location_ = check, .data_size_ = patch_area
			};

			instruction_cache.store();
			code_healers_original_instruction_cache.push_back(instruction_cache);

			utils::hook::nop(check, patch_area, true);
			utils::hook::call(check, stub);
			instruction_cache.store();
			code_healers_instruction_cache.push_back(instruction_cache);

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
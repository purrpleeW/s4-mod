#include "common.hpp"
#include "component/arxan.hpp"

#include "game/game.hpp"

#include <component/scheduler.hpp>
#include <loader/component_loader.hpp>
#include <utils/hook.hpp>

#include <ranges>

namespace arxan {
	using namespace asmjit::x86;

	namespace {
		std::vector<void*> Arxan_IntegrityChecks_Intact{};
		std::vector<void*> Arxan_IntegrityChecks_IntactBig{};
		std::vector<void*> Arxan_IntegrityChecks_Split{};
		std::vector<void*> Arxan_HealingLocations_Type0{};
		std::vector<void*> Arxan_HealingLocations_Type1{};
		std::vector<void*> Arxan_HealingLocations_Type2{};
		std::vector<void*> Arxan_HealingLocations_Type3{};

		HANDLE arxan_thread_handle = nullptr;

		constexpr std::uint32_t reverse_bytes(const std::uint32_t x) noexcept {
			return std::byteswap(x);
		}

		auto transform_checksum(const checksum_location_type chk_type, const std::size_t size) noexcept {
			return std::views::transform([chk_type, size](const auto& location) noexcept {
				return std::tuple{location, chk_type, size};
			});
		};

		bool arxan_healing_checksum(std::uintptr_t rbp) {
			std::uintptr_t rbp_address_location_ptr = DEREF_PTR_AS(std::uintptr_t, rbp + 0x10);

			for (auto& checksum : stored_checksum_locations) {
				// TODO: if 0x7 is too big then "mov [rdx], al" will make the game crash probably because its trying to overwrite areas next to our hooks that
				//       have to get modified.
				// we could do two seperate functions since "mov [rdx], eax" would be a 32 byte offset (?) and "mov [rdx], al" would be 4 byte offset (?)
				if (rbp_address_location_ptr + 0x7 >= checksum.location_.address_ && rbp_address_location_ptr - 0x7 <= checksum.location_.address_) {
					return true;
				}
			}

			return false;
		}

		void arxan_healing_type0_handler(asmjit::x86::Assembler& a, asmjit::Label& label_l1, std::uintptr_t) {
			a.mov(qword_ptr(rdx), eax);			// mov	[rdx], eax

			a.bind(label_l1);
			a.mov(eax, qword_ptr(rbp, 0x20));	// mov	eax, [rbp+20h]
		}

		void arxan_healing_type1_handler(asmjit::x86::Assembler& a, asmjit::Label& label_l1, std::uintptr_t) {
			a.mov(qword_ptr(rdx), al);			// mov	[rdx], al

			a.bind(label_l1);
			a.add(dword_ptr(rbp, 0x20), -1);	// add	dword ptr [rbp+20h], -1
		}

		void arxan_healing_type2_handler(asmjit::x86::Assembler& a, asmjit::Label& label_l1, std::uintptr_t jmp_location) {
			a.mov(qword_ptr(rdx), eax);			// mov	[rdx], eax

			a.bind(label_l1);
			a.add(rsp, 0x8);
			a.mov(r15, jmp_location);			// jmp	{jmp location} ; this is not a literal jmp instruction, but functions as one
			a.push(r15);
		}

		void arxan_healing_type3_handler(asmjit::x86::Assembler& a, asmjit::Label& label_l1, std::uintptr_t jmp_location) {
			a.mov(qword_ptr(rdx), al);			// mov	[rdx], al

			a.bind(label_l1);
			a.add(rsp, 0x8);
			a.mov(r15, jmp_location);			// jmp	{jmp location} ; this is not a literal jmp instruction, but functions as one
			a.push(r15);
		}

		int fix_checksum(std::uint64_t rbp_offset, std::uint64_t ptr_offset, std::uint64_t* ptr_stack, std::uint32_t jmp_instruction_dist,
			std::uint32_t calculated_checksum_from_arg)
		{
			static utils::nt::library game{};

			// check if our checksum hooks got overwritten
			// we could probably ifdef this now but it's a good indicator to know if our checksum hooks still exist
			for (auto& checksum : stored_checksum_locations) {
				if (memcmp(checksum.location_.pointer_, checksum.stored_data_.data(), checksum.stored_data_.size()) == 0) {
					continue;
				}
				utils::hook::copy(checksum.location_.address_, checksum.stored_data_.data(), checksum.stored_data_.size(), true);
			}

			// 0x120 is a good starting point to decrement downwards to find the calculated checksum on the stack

			std::uint32_t calculated_checksum = calculated_checksum_from_arg;
			std::uint32_t reversed_checksum = reverse_bytes(calculated_checksum_from_arg);
			std::uint32_t* calculated_checksum_ptr = PTR_AS(std::uint32_t*, PTR_AS(std::uintptr_t, ptr_stack) + 0x120);
			std::uint32_t* calculated_reversed_checksum_ptr = PTR_AS(std::uint32_t*, PTR_AS(std::uintptr_t, ptr_stack) + 0x120);

			bool double_text_checksum = false;
			std::uintptr_t* previous_result_ptr = nullptr;
			if (ptr_offset == 0 && rbp_offset < 0x90) {
				// make sure rbpOffset is aligned by 8 bytes
				std::uintptr_t* text_ptr = PTR_AS(std::uintptr_t*, PTR_AS(std::uintptr_t, ptr_stack) + rbp_offset + (rbp_offset % 0x8));
				int pointer_counter = 0;

				for (int ix = 0; ix < 20; ix++) {
					std::uintptr_t deref_ptr = *text_ptr;
					if (game.is_address_in_range(deref_ptr)) {
						std::uintptr_t deref_result = *DEREF_PTR_AS(uintptr_t*, text_ptr);
						pointer_counter++;
						if (deref_result == 0xFFFFFFFFFFFFFFFF) {
							if (pointer_counter > 2) {
								double_text_checksum = true;
								text_ptr++;
							}
							break;
						}
						previous_result_ptr = text_ptr;
					}

					text_ptr--;
				}
			}

			for (int ix = 0; ix < 80; ix++) {
				std::uint32_t deref_ptr = DEREF_PTR_AS(std::uint32_t, calculated_checksum_ptr);
				if (deref_ptr == calculated_checksum) {
					break;
				}

				calculated_checksum_ptr--;
			}

			for (int ix = 0; ix < 80; ix++) {
				std::uint32_t deref_ptr = DEREF_PTR_AS(std::uint32_t, calculated_reversed_checksum_ptr);
				if (deref_ptr == reversed_checksum) {
					break;
				}

				calculated_reversed_checksum_ptr--;
			}

			// add remainder to align ptr
			std::uintptr_t* text_ptr = PTR_AS(std::uintptr_t*, PTR_AS(std::uintptr_t, ptr_stack) + rbp_offset + (rbp_offset % 0x8));
			std::uint32_t original_checksum = 0;
			std::uint32_t* original_checksum_ptr = nullptr;

			// searching for a .text pointer that points to the original checksum, upwards from the rbp
			for (int ix = 0; ix < 10; ix++, text_ptr--) {
				std::uintptr_t deref_ptr = *text_ptr;

				if (!game.is_address_in_range(deref_ptr)) {
					continue;
				}

				if (ptr_offset == 0 && rbp_offset < 0x90) {
					original_checksum = double_text_checksum
						? *DEREF_PTR_AS(std::uint32_t*, previous_result_ptr)
						: DEREF_PTR_AS(std::uint32_t, deref_ptr)
					;

					break;
				}

				// if ptrOffset is used the original checksum is in a different spot
				original_checksum_ptr = PTR_AS(std::uint32_t*, deref_ptr + ptr_offset * 4);
				original_checksum = *original_checksum_ptr;

				break;
			}

			*calculated_checksum_ptr = original_checksum;
			*calculated_reversed_checksum_ptr = reverse_bytes(original_checksum);

			if (original_checksum_ptr != nullptr) {
				std::uint32_t* tmp_original_checksum_ptr = original_checksum_ptr;
				std::uint32_t* tmp_calculated_checksum_ptr = calculated_checksum_ptr;
				std::uint32_t* tmp_reversed_checksum_ptr = calculated_reversed_checksum_ptr;

				for (auto x = 0; x <= ptr_offset; x++) {
					*tmp_calculated_checksum_ptr = *tmp_original_checksum_ptr;
					*tmp_reversed_checksum_ptr = reverse_bytes(*tmp_original_checksum_ptr);

					tmp_original_checksum_ptr--;
					tmp_calculated_checksum_ptr--;
					tmp_reversed_checksum_ptr--;
				}
			}

			return original_checksum;
		}

		void create_inline_asm_stub() {
			static utils::nt::library game{};

			Arxan_IntegrityChecks_Split.erase(std::remove_if(Arxan_IntegrityChecks_Split.begin(), Arxan_IntegrityChecks_Split.end(), [](void* addr) {
				auto res = memory::scanned_result<void*>(addr);
				auto jmp_bc = res.add(3).add(1).rip().as<std::uint8_t*>();
				if (game.is_address_in_range(PTR_AS(std::uintptr_t, jmp_bc)) && jmp_bc[0] == 0x83
					&& (jmp_bc[1] == 0x85 || (jmp_bc[1] == 0x45 && jmp_bc[3] == 0xFF)))
				{
					return false;
				}
				return true;
			}), Arxan_IntegrityChecks_Split.end());

			static constexpr auto allocation_size = 0x4000;
			void* asm_big_stub_location = utils::hook::allocate_somewhere_near(game.get_ptr(), allocation_size);
			utils::hook::nop(asm_big_stub_location, allocation_size);

			char* previous_stub_offset = nullptr;	// avoid stub generation collision
			char* current_stub_offset = nullptr;	// for jmp distance calculation

			auto locations = std::views::join(std::array{
				Arxan_IntegrityChecks_Intact    | transform_checksum(checksum_location_type::Intact, 7),
				Arxan_IntegrityChecks_IntactBig | transform_checksum(checksum_location_type::IntactBig, 10),
				Arxan_IntegrityChecks_Split     | transform_checksum(checksum_location_type::Split, 8)
			});

			for (auto&& [raw_address, type, size] : locations) {
				auto location = checksum_stack_data{
					.location_  = raw_address,
					.type_      = type,
					.data_size_ = size
				};

				// we don't know the previous offset yet
				if (current_stub_offset == nullptr) {
					current_stub_offset = PTR_AS(char*, asm_big_stub_location);
				}
				if (previous_stub_offset != nullptr) {
					current_stub_offset = previous_stub_offset;
				}

				int jmp_distance = 0;
				std::uintptr_t rbp_offset = 0;

				auto location_res = memory::scanned_result<void>(location.location_.address_);
				if (location.type_ == checksum_location_type::Split) {
					jmp_distance = *location_res.add(4).as<int*>();
					std::uint8_t* rbp_offset_ptr = location_res.add(4).rip().as<std::uint8_t*>();

					// depending on the rbp offset from add dword ptr we need one more byte for the rbpOffset
					if (rbp_offset_ptr[1] == 0x45) {
						// add dword ptr [rbp+68],-01
						rbp_offset = DEREF_PTR_AS(char, &rbp_offset_ptr[2]);
					}
					else if (rbp_offset_ptr[1] == 0x85) {
						// add dword ptr [rbp+1CC],-01
						rbp_offset = DEREF_PTR_AS(short, &rbp_offset_ptr[2]);
					}
				}

				static asmjit::JitRuntime runtime;
				asmjit::CodeHolder code;
				code.init(runtime.environment());

				asmjit::x86::Assembler a(&code);

				if (location.type_ != checksum_location_type::Split) {
					rbp_offset = location.location_.pointer_bytes_[5];
				}

				a.sub(rsp, 0x32);
				asm_impl::push_ad64(&a);

				a.mov(qword_ptr(rsp, 0x20), rax);
				a.mov(rdx, rcx);	// offset within text section pointer (ecx*4)

				// we dont use rbpoffset since we only get 1 byte from the 2 byte offset (rbpOffset)
				// 0x130 is a good starting ptr to decrement downwards so we can find the original checksum
				if (location.type_ == checksum_location_type::IntactBig) {
					a.mov(rcx, 0x120);
				}
				else {
					a.mov(rcx, rbp_offset);
				}

				a.mov(r8, rbp);
				a.mov(r9, jmp_distance);
				a.mov(rax, PTR_AS(void*, fix_checksum));
				a.call(rax);
				a.add(rsp, 0x8 * 4); // so that r12-r15 registers dont get corrupt

				asm_impl::pop_ad64_no_rax(&a);
				a.add(rsp, 0x32);

				a.mov(ptr(rdx, rcx, 2), eax); // mov [rdx+rcx*4], eax

				if (jmp_distance == 0) {
					if (location.type_ == checksum_location_type::IntactBig) {
						rbp_offset += 0x100;
					}
					// add dword ptr [rbp+rbpOffset], 0FFFFFFFFh
					a.add(dword_ptr(rbp, rbp_offset), -1);
				}
				else {
					// jmp loc_7FF641C707A5
					// push the desired address on to the stack and then perform a 64 bit RET
					a.add(rsp, 0x8); // pop return address off the stack cause we will jump
					std::uintptr_t address_to_jump = location.location_.address_ + jmp_distance;
					if (location.type_ == checksum_location_type::Split) {
						// 0x8 call instruction + offset + 2 nops
						address_to_jump += 0x8;
					}
					a.mov(r11, address_to_jump);	// r11 is being used but should be fine based on documentation
					if (location.type_ == checksum_location_type::Split) {
						// since we dont pop off rax we need to sub 0x8 the rsp
						a.add(rsp, 0x8);
					}

					a.push(r11);
				}

				if (location.type_ != checksum_location_type::Split) {
					// since we dont pop off rax we need to sub 0x8 the rsp
					a.add(rsp, 0x8);
				}

				a.ret();

				void* asmjit_result = nullptr;
				runtime.add(&asmjit_result, &code);

				const auto stub_size = code.codeSize();
				utils::hook::copy(current_stub_offset, asmjit_result, stub_size);
				utils::hook::nop(location.location_.address_, location.data_size_);
				utils::hook::call(location.location_.address_, current_stub_offset);
				location.store();

				stored_checksum_locations.push_back(location);

				previous_stub_offset = current_stub_offset + stub_size + 0x8;
			}

			const auto intact_count = Arxan_IntegrityChecks_Intact.size();
			const auto intact_big_count = Arxan_IntegrityChecks_IntactBig.size();
			const auto split_count = Arxan_IntegrityChecks_Split.size();
			const auto total_count = intact_count + intact_big_count + split_count;

			LOG("Component/Arxan", DEBUG, "Finna bridge {} checksum hooks. [{} intact, {} intact big, {} split]", total_count, intact_count, intact_big_count,
				split_count);
		}

		void create_checksum_healing_stub() {
			static utils::nt::library game{};
			static std::size_t patched_code_healers = 0;

			checksum_healing_location healing_locations[] = {
				{ .pattern = Arxan_HealingLocations_Type0, .length = 5, .handler = arxan_healing_type0_handler },
				{ .pattern = Arxan_HealingLocations_Type1, .length = 6, .handler = arxan_healing_type1_handler },
				{ .pattern = Arxan_HealingLocations_Type2, .length = 7, .handler = arxan_healing_type2_handler },
				{ .pattern = Arxan_HealingLocations_Type3, .length = 7, .handler = arxan_healing_type3_handler },
			};

			static constexpr std::size_t allocation_size = 0x3E800;
			void* healing_stub_location = utils::hook::allocate_somewhere_near(game.get_ptr(), allocation_size);
			utils::hook::nop(healing_stub_location, allocation_size, true);

			char* previous_stub_offset = nullptr;	// avoid stub generation collision
			char* current_stub_offset = nullptr;	// for jmp distance calculation

			for (auto [loc_type, entry] : std::views::enumerate(healing_locations)) {
				auto& [checksum_pattern, checksum_length, checksum_handler] = entry;
				for (auto& loc_address : checksum_pattern) {
					std::uintptr_t jmp_location = 0;

					// we don't know the previous offset yet
					if (current_stub_offset == nullptr) {
						current_stub_offset = PTR_AS(char*, healing_stub_location);
					}
					if (previous_stub_offset != nullptr) {
						current_stub_offset = previous_stub_offset;
					}

					auto location = memory::scanned_result(loc_address);
					if (location.add(2).is(0xE9 /* jmp */)) {
						jmp_location = location.add(3).rip().as<std::uintptr_t>();

						if (!game.is_address_in_accessible_range(jmp_location)) {
							continue;
						}

						if (loc_type == 2 || loc_type == 3) {
							static constexpr std::uint8_t instruction_loc2[] = { 0x8B, 0x45, 0x20 };
							static constexpr std::uint8_t instruction_loc3[] = { 0x8B, 0x45, 0x20, 0xFF };

							if (std::memcmp(PTR_AS(void*, jmp_location),
								(loc_type == 2) ? instruction_loc2 : instruction_loc3,
								(loc_type == 2) ? sizeof(instruction_loc2) : sizeof(instruction_loc2)
							) != 0) {
								continue;
							}
						}
					}

					static asmjit::JitRuntime runtime;
					asmjit::CodeHolder code;
					code.init(runtime.environment());

					asmjit::x86::Assembler a(&code);
					asmjit::Label label_l1 = a.newLabel();

					a.sub(rsp, 0x32);
					asm_impl::push_ad64_min(&a);

					a.mov(rcx, rbp);
					a.mov(r15, PTR_AS(void*, arxan_healing_checksum));
					a.call(r15);
					a.movzx(r15, al);	// if arxan tries to replace our checksum set r15 to 1

					asm_impl::pop_ad64_min(&a);
					a.add(rsp, 0x32);
					a.cmp(r15, 1);
					a.je(label_l1);

					if (checksum_handler != nullptr) {
						checksum_handler(a, label_l1, jmp_location);
					}

					a.ret();

					void* asmjit_result = nullptr;
					runtime.add(&asmjit_result, &code);

					const auto stub_size = code.codeSize();
					utils::hook::copy(current_stub_offset, asmjit_result, stub_size, true);
					utils::hook::nop(location.get(), checksum_length, true);
					utils::hook::call(location.get(), current_stub_offset);

					previous_stub_offset = current_stub_offset + stub_size + 0x8;
					patched_code_healers++;
				}
			}

			LOG("Component/Arxan", DEBUG, "Opp remover technology eliminated {} snitch{}.", patched_code_healers, patched_code_healers == 1 ? "" : "es");
		}
	}

	namespace asm_impl {
		void push_ad64(asmjit::x86::Assembler* a) {
			push_ad64_min(a);
			a->push(r12);
			a->push(r13);
			a->push(r14);
			a->push(r15);
		}

		void pop_ad64(asmjit::x86::Assembler* a) {
			a->pop(r15);
			a->pop(r14);
			a->pop(r13);
			a->pop(r12);
			pop_ad64_min(a);
		}

		void pop_ad64_no_rax(asmjit::x86::Assembler* a) {
			a->pop(r11);
			a->pop(r10);
			a->pop(r9);
			a->pop(r8);
			a->pop(rdi);
			a->pop(rsi);
			a->pop(rbp);
			a->pop(rsp);
			a->pop(rbx);
			a->pop(rdx);
			a->pop(rcx);
		}

		void push_ad64_min(asmjit::x86::Assembler* a) {
			a->push(rax);
			a->push(rcx);
			a->push(rdx);
			a->push(rbx);
			a->push(rsp);
			a->push(rbp);
			a->push(rsi);
			a->push(rdi);
			a->push(r8);
			a->push(r9);
			a->push(r10);
			a->push(r11);
		}

		void pop_ad64_min(asmjit::x86::Assembler* a) {
			pop_ad64_no_rax(a);
			a->pop(rax);
		}
	}

	struct component final : generic_component {
		component_priority priority() const override {
			return component_priority::arxan;
		}

		void find_signatures(memory::signature_store& batch) override {
			batch.add(SETUP_POINTER_V(Arxan_IntegrityChecks_Intact), "89 04 8A 83 45 ? FF");
			batch.add(SETUP_POINTER_V(Arxan_IntegrityChecks_Split), "89 04 8A E9");

			if (!identification::game::is_config(identification::game::Beta)) {
				batch.add(SETUP_POINTER_V(Arxan_IntegrityChecks_IntactBig), "89 04 8A 83 85");
			}

			batch.add(SETUP_POINTER_V(Arxan_HealingLocations_Type0), "89 02 8B 45 20");
			batch.add(SETUP_POINTER_V(Arxan_HealingLocations_Type1), "88 02 83 45 20 FF");

			if (!identification::game::is_mode(identification::game::CDL)
				&& !identification::game::is_platform(identification::game::Steam)
			) {
				batch.add(SETUP_POINTER_V(Arxan_HealingLocations_Type2), "89 02 E9");
				batch.add(SETUP_POINTER_V(Arxan_HealingLocations_Type3), "88 02 E9");
			}
		}

		void post_load() override {
			scheduler::once<scheduler::pipeline::client>([] {
				arxan_thread_handle = CreateThread(nullptr, 0, [](PVOID) -> DWORD {
					using namespace std::chrono;

					const auto timepoint = steady_clock::now();
					{
						create_inline_asm_stub();
						create_checksum_healing_stub();
					}
					const auto elapsed_time = duration_cast<milliseconds>(steady_clock::now() - timepoint).count();

					LOG("Component/Arxan", DEBUG, "Created inline hooks for checksums in {}ms.", elapsed_time);
					return 0;
				}, nullptr, 0, 0);
			});
		}

		void pre_destroy() override {
			CloseHandle(arxan_thread_handle);
		}
	};
}

REGISTER_COMPONENT(arxan::component)

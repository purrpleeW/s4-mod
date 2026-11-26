#include "common_core.hpp"

#include "utils/hook.hpp"

#include <map>
#include <MinHook.h>
#include "ci/ci.hpp"
#include "logger/logger.hpp"
#include "utils/concurrency.hpp"
#include "utils/nt.hpp"
#include "utils/string.hpp"

#ifdef max
#	undef max
#endif

#ifdef min
#	undef min
#endif

namespace utils::hook {
	namespace {
		class memory {
		public:
			memory() = default;

			memory(const void* ptr)
				: memory()
			{
				static const auto allocation_granularity = get_allocation_granularity();
				this->length_ = allocation_granularity;

				this->buffer_ = allocate_somewhere_nearish(ptr, allocation_granularity, this->length_);
				if (!this->buffer_) {
					throw std::runtime_error("Failed to allocate");
				}
			}

			~memory() {
				if (this->buffer_) {
					VirtualFree(this->buffer_, 0, MEM_RELEASE);
				}
			}

			memory(memory&& obj) noexcept
				: memory()
			{
				this->operator=(std::move(obj));
			}

			memory& operator=(memory&& obj) noexcept {
				if (this != &obj) {
					this->~memory();
					this->buffer_ = obj.buffer_;
					this->length_ = obj.length_;
					this->offset_ = obj.offset_;

					obj.buffer_ = nullptr;
					obj.length_ = 0;
					obj.offset_ = 0;
				}

				return *this;
			}

			void* allocate(const std::size_t length) {
				if (!this->buffer_) {
					return nullptr;
				}

				if (this->offset_ + length > this->length_) {
					return nullptr;
				}

				const auto ptr = this->get_ptr();
				this->offset_ += length;
				return ptr;
			}

			void* get_ptr() const {
				return this->buffer_ + this->offset_;
			}

		private:
			std::uint8_t* buffer_{};
			std::size_t length_{};
			std::size_t offset_{};
		};

		void* get_memory_near(const void* address, const std::size_t size) {
			static concurrency::container<std::vector<memory>> memory_container{};

			return memory_container.access<void*>([&](std::vector<memory>& memories) {
				for (auto& memory : memories) {
					if (!is_relatively_far(address, memory.get_ptr())) {
						const auto buffer = memory.allocate(size);
						if (buffer) {
							return buffer;
						}
					}
				}

				memories.emplace_back(address);
				return memories.back().allocate(size);
			});
		}

		void* initialize_min_hook() {
			static class min_hook_init {
			public:
				min_hook_init() {
					if (MH_Initialize() != MH_OK) {
						throw std::runtime_error("Failed to initialize MinHook");
					}
				}

				~min_hook_init() {
					MH_Uninitialize();
				}
			} min_hook_init;
			return &min_hook_init;
		}
	}

	void assembler::pushad64() {
		this->pushad64_min();
		this->push(r12);
		this->push(r13);
		this->push(r14);
		this->push(r15);
	}

	void assembler::pushad64_min() {
		this->push(rax);
		this->push(rcx);
		this->push(rdx);
		this->push(rbx);
		this->push(rsp);
		this->push(rbp);
		this->push(rsi);
		this->push(rdi);
		this->push(r8);
		this->push(r9);
		this->push(r10);
		this->push(r11);
	}

	void assembler::popad64() {
		this->pop(r15);
		this->pop(r14);
		this->pop(r13);
		this->pop(r12);
		this->popad64_min();
	}

	void assembler::popad64_no_rax() {
		this->pop(r11);
		this->pop(r10);
		this->pop(r9);
		this->pop(r8);

		this->pop(rdi);
		this->pop(rsi);
		this->pop(rbp);
		this->pop(rsp);
		this->pop(rbx);
		this->pop(rdx);
		this->pop(rcx);
	}

	void assembler::popad64_min() {
		this->popad64_no_rax();
		this->pop(rax);
	}

	void assembler::prepare_stack_for_call() {
		const auto reserve_callee_space = this->newLabel();
		const auto stack_unaligned = this->newLabel();

		this->test(rsp, 0xF);
		this->jnz(stack_unaligned);

		this->sub(rsp, 0x8);
		this->push(rsp);

		this->push(rax);
		this->mov(rax, ptr(rsp, 8, 8));
		this->add(rax, 0x8);
		this->mov(ptr(rsp, 8, 8), rax);
		this->pop(rax);

		this->jmp(reserve_callee_space);

		this->bind(stack_unaligned);
		this->push(rsp);

		this->bind(reserve_callee_space);
		this->sub(rsp, 0x40);
	}

	void assembler::restore_stack_after_call() {
		this->lea(rsp, ptr(rsp, 0x40));
		this->pop(rsp);
	}

	asmjit::Error assembler::call(void* target) {
		return Assembler::call(reinterpret_cast<std::size_t>(target));
	}

	asmjit::Error assembler::jmp(void* target) {
		return Assembler::jmp(reinterpret_cast<std::size_t>(target));
	}

	detour::detour() {
		(void)initialize_min_hook();
	}

	detour::detour(const std::size_t place, void* target)
		: detour(reinterpret_cast<void*>(place), target) {
	}

	detour::detour(void* place, void* target)
		: detour() {
		this->create(place, target);
	}

	detour::~detour() {
		this->clear();
	}

	void detour::enable() {
		if (MH_EnableHook(this->place_) != MH_OK) {
			if (ci::is_testing()) {
				ci::record_test_state(ci::Failure, ci::Hook, "detour::enable() -> minhook", string::va("failure at %p (origin: %s)",
					this->place_, and_rel(PTR_AS(std::uintptr_t, _ReturnAddress())).c_str()));
			}
			else {
				throw std::runtime_error(string::va("Unable to enable hook at location: %p (Origin: %s)", this->place_,
					and_rel(PTR_AS(std::uintptr_t, _ReturnAddress())).c_str()));
			}
		}
		else {
			ci::record_test_state(ci::InProgress, ci::Hook, "detour::enable() -> minhook", string::va("hook at %p without issue (origin: %s)",
					this->place_, and_rel(PTR_AS(std::uintptr_t, _ReturnAddress())).c_str()));
		}

		if (!this->moved_data_.empty()) {
			this->move();
		}
	}

	void detour::disable() {
		this->un_move();
		MH_DisableHook(this->place_);
	}

	void detour::create(void* place, void* target) {
		// LOG("Detours", DEBUG, "creating {} from {}", and_rel(PTR_AS(std::uintptr_t, place)), and_rel(PTR_AS(std::uintptr_t, _ReturnAddress())));
		this->clear();
		this->place_ = place;

		if (MH_CreateHook(this->place_, target, &this->original_) != MH_OK) {
			if (!ci::is_testing()) {
				throw std::runtime_error(string::va("Unable to create hook at location: %p (Origin: %s)", this->place_,
					and_rel(PTR_AS(std::uintptr_t, _ReturnAddress())).c_str()));
			}
		}

		this->enable();
	}

	void detour::create(const std::size_t place, void* target) {
		this->create(reinterpret_cast<void*>(place), target);
	}

	void detour::create(const std::string& target_library, const std::string& target_process, void* target) {
		utils::nt::library library(target_library);
		if (!library.is_valid()) {
			return;
		}

		auto proc = library.get_proc<void*>(target_process);
		if (proc == nullptr) {
			return;
		}

		this->create(proc, target);
	}

	void detour::clear() {
		if (this->place_) {
			this->un_move();
			MH_RemoveHook(this->place_);
		}

		this->place_ = nullptr;
		this->original_ = nullptr;
		this->moved_data_ = {};
	}

	void detour::move() {
		this->moved_data_ = move_hook(this->place_);
	}

	void* detour::get_place() const {
		return this->place_;
	}

	void* detour::get_original() const {
		return this->original_;
	}

	void detour::un_move() {
		if (!this->moved_data_.empty()) {
			copy(this->place_, this->moved_data_.data(), this->moved_data_.size());
		}
	}

	iat_detour::iat_detour(const utils::nt::library& library, const std::string& target_library, const std::string& process, void* target)
		: iat_detour()
	{
		this->create(library, target_library, process, target);
	}

	iat_detour::~iat_detour() {
		this->clear();
	}

	void iat_detour::enable() {
		if (this->place_ == nullptr) {
			return;
		}

		this->original_ = DEREF_PTR_AS(void*, this->place_);
		set(this->place_, this->target_);

		ci::record_test_state(ci::InProgress, ci::Hook, "iat_detour::enable()", string::va("hook at %p (origin: %s)",
			this->place_, and_rel(PTR_AS(std::uintptr_t, _ReturnAddress())).c_str()));
	}

	void iat_detour::disable() {
		if (this->place_ == nullptr) {
			return;
		}

		set(this->place_, this->original_);
	}

	void iat_detour::create(const std::string& target_library, const std::string& process, void* target) {
		return create({}, target_library, process, target);
	}

	void iat_detour::create(const utils::nt::library& library, const std::string& target_library, const std::string& process, void* target) {
		this->clear();
		this->place_ = library.get_iat_entry(target_library, process);
		if (this->place_ == nullptr) {
			return;
		}

		this->target_ = target;
		this->enable();
	}

	void iat_detour::clear() {
		if (this->place_) {
			this->disable();
		}

		this->place_ = nullptr;
		this->original_ = nullptr;
	}

	void* iat_detour::get_place() const {
		return this->place_;
	}

	void* iat_detour::get_original() const {
		return this->original_;
	}

	std::size_t get_allocation_granularity() {
		SYSTEM_INFO info{};
		GetSystemInfo(&info);

		return info.dwAllocationGranularity;
	}

	std::uint8_t* allocate_somewhere_nearish(const void* base_address, const std::size_t granularity, const std::size_t size) {
		std::size_t target_address = reinterpret_cast<std::size_t>(base_address) - (1ull << 31);
		target_address &= ~(granularity - 1);

		while (true) {
			target_address += granularity;

			auto* target_ptr = reinterpret_cast<std::uint8_t*>(target_address);
			if (is_relatively_far(base_address, target_ptr)) {
				return nullptr;
			}

			const auto res = VirtualAlloc(target_ptr, size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			if (res) {
				if (is_relatively_far(base_address, target_ptr)) {
					VirtualFree(res, 0, MEM_RELEASE);
					return nullptr;
				}

				return static_cast<std::uint8_t*>(res);
			}
		}
	}

	std::uint8_t* allocate_somewhere_near(const void* base_address, const std::size_t size) {
		std::size_t offset = 0;

		while (true) {
			offset += size;

			auto* target_address = static_cast<const std::uint8_t*>(base_address) - offset;
			if (is_relatively_far(base_address, target_address)) {
				return nullptr;
			}

			const auto res = VirtualAlloc(const_cast<std::uint8_t*>(target_address), size, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
			if (res) {
				if (is_relatively_far(base_address, target_address)) {
					VirtualFree(res, 0, MEM_RELEASE);
					return nullptr;
				}

				return static_cast<std::uint8_t*>(res);
			}
		}
	}

	std::optional<std::pair<void*, void*>> iat(const nt::library& library, const std::string& target_library, const std::string& process, void* stub) {
		if (!library.is_valid()) {
			return {};
		}

		auto* const ptr = library.get_iat_entry(target_library, process);
		if (!ptr) {
			return {};
		}

		DWORD protect;
		VirtualProtect(ptr, sizeof(*ptr), PAGE_EXECUTE_READWRITE, &protect);

		std::swap(*ptr, stub);

		VirtualProtect(ptr, sizeof(*ptr), protect, &protect);
		return {{ ptr, stub }};
	}

	void nop(void* place, const std::size_t length, bool safe) {
		DWORD old_protect{};
		VirtualProtect(place, length, PAGE_EXECUTE_READWRITE, &old_protect);

		std::memset(place, 0x90, length);

		VirtualProtect(place, length, old_protect, &old_protect);
		FlushInstructionCache(GetCurrentProcess(), place, length);

		if (!safe && ci::is_testing()) {
			ci::record_test_state(ci::InProgress, ci::Hook, "utils::hook::nop", string::va("at %p (origin: %s)",
				place, and_rel(PTR_AS(std::uintptr_t, _ReturnAddress())).c_str()));
		}
	}

	void nop(const std::size_t place, const std::size_t length, bool safe) {
		nop(reinterpret_cast<void*>(place), length, safe);
	}

	void copy(void* place, const void* data, const std::size_t length, bool safe) {
		DWORD old_protect{};
		VirtualProtect(place, length, PAGE_EXECUTE_READWRITE, &old_protect);

		std::memmove(place, data, length);

		VirtualProtect(place, length, old_protect, &old_protect);
		FlushInstructionCache(GetCurrentProcess(), place, length);

		if (!safe && ci::is_testing()) {
			ci::record_test_state(ci::InProgress, ci::Hook, "utils::hook::copy", string::va("hook at %p (origin: %s)",
				place, and_rel(PTR_AS(std::uintptr_t, _ReturnAddress())).c_str()));
		}
	}

	void copy(const std::size_t place, const void* data, const std::size_t length, bool safe) {
		copy(reinterpret_cast<void*>(place), data, length, safe);
	}

	void copy_string(void* place, const char* str, bool safe) {
		copy(reinterpret_cast<void*>(place), str, strlen(str) + 1, safe);
	}

	void copy_string(const std::size_t place, const char* str, bool safe) {
		copy_string(reinterpret_cast<void*>(place), str, safe);
	}

	bool is_relatively_far(const void* pointer, const void* data, const int offset) {
		return is_relatively_far(reinterpret_cast<std::size_t>(pointer), reinterpret_cast<std::size_t>(data), offset);
	}

	bool is_relatively_far(const std::size_t pointer, const std::size_t data, const int offset) {
		const auto diff = static_cast<std::int64_t>(data - (pointer + offset));
		const auto small_diff = static_cast<std::int32_t>(diff);
		return diff != static_cast<std::int64_t>(small_diff);
	}

	void call(void* pointer, void* data) {
#		if defined(_WIN64)
			if (is_relatively_far(pointer, data)) {
				auto* trampoline = get_memory_near(pointer, 14);
				if (!trampoline) {
					throw std::runtime_error("Too far away to create 32bit relative branch");
				}

				call(pointer, trampoline);
				jump(trampoline, data, true, true);
				return;
			}
#		endif

		std::uint8_t copy_data[5];
		copy_data[0] = 0xE8;
		*reinterpret_cast<std::int32_t*>(&copy_data[1]) = static_cast<std::int32_t>(reinterpret_cast<std::size_t>(data)
			- (reinterpret_cast<std::size_t>(pointer) + 5));

		auto* patch_pointer = static_cast<PBYTE>(pointer);
		copy(patch_pointer, copy_data, sizeof(copy_data));
	}

	void call(const std::size_t pointer, void* data) {
		return call(reinterpret_cast<void*>(pointer), data);
	}

	void call(const std::size_t pointer, const std::size_t data) {
		return call(pointer, reinterpret_cast<void*>(data));
	}

	void jump(void* pointer, void* data, const bool use_far, const bool use_safe) {
#		if defined(_WIN64)
			static const unsigned char jump_data[] = {
				0x48, 0xB8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, 0xFF, 0xE0
			};

			static const unsigned char jump_data_safe[] = {
				0xFF, 0x25, 0x00, 0x00, 0x00, 0x00
			};

			if (!use_far && is_relatively_far(pointer, data)) {
				auto* trampoline = get_memory_near(pointer, 14);
				if (!trampoline) {
					throw std::runtime_error("Too far away to create 32bit relative branch");
				}
				jump(pointer, trampoline, false, false);
				jump(trampoline, data, true, true);
				return;
			}
#		endif

			auto* patch_pointer = PBYTE(pointer);

#		if defined(_WIN64)
			if (use_far) {
				if (use_safe) {
					std::uint8_t copy_data[sizeof(jump_data_safe) + sizeof(data)];
					memcpy(copy_data, jump_data_safe, sizeof(jump_data_safe));
					memcpy(copy_data + sizeof(jump_data_safe), &data, sizeof(data));

					copy(patch_pointer, copy_data, sizeof(copy_data));
				}
				else {
					std::uint8_t copy_data[sizeof(jump_data)];
					memcpy(copy_data, jump_data, sizeof(jump_data));
					memcpy(copy_data + 2, &data, sizeof(data));

					copy(patch_pointer, copy_data, sizeof(copy_data));
				}
			}
			else {
#		endif
				std::uint8_t copy_data[5];
				copy_data[0] = 0xE9;
				*reinterpret_cast<std::int32_t*>(&copy_data[1]) = std::int32_t(std::size_t(data) - (std::size_t(pointer) + 5));

				copy(patch_pointer, copy_data, sizeof(copy_data));
#		if defined(_WIN64)
			}
#		endif
	}

	void jump(const std::size_t pointer, void* data, const bool use_far, const bool use_safe) {
		return jump(reinterpret_cast<void*>(pointer), data, use_far, use_safe);
	}

	void jump(const std::size_t pointer, const std::size_t data, const bool use_far, const bool use_safe) {
		return jump(pointer, reinterpret_cast<void*>(data), use_far, use_safe);
	}

	std::pair<void*, std::size_t> assemble(const std::function<void(assembler&)>& asm_function) {
		static asmjit::JitRuntime runtime;

		asmjit::CodeHolder code;
		code.init(runtime.environment());

		assembler a(&code);

		asm_function(a);

		void* result = nullptr;
		runtime.add(&result, &code);

		return std::make_pair(result, code.codeSize());
	}

	void inject(std::size_t pointer, std::size_t data) {
		if (is_relatively_far(pointer, data, 4)) {
			throw std::runtime_error("Too far away to create 32bit relative branch");
		}

		set<std::int32_t>(pointer, static_cast<std::int32_t>(data - (pointer + 4)));
	}

	void inject(void* pointer, const void* data) {
		return inject(reinterpret_cast<std::size_t>(pointer), reinterpret_cast<std::size_t>(data));
	}

	void inject(const std::size_t pointer, const void* data) {
		return inject(pointer, reinterpret_cast<std::size_t>(data));
	}

	std::vector<std::uint8_t> move_hook(void* pointer) {
		std::vector<std::uint8_t> original_data{};

		auto* data_ptr = static_cast<std::uint8_t*>(pointer);
		if (data_ptr[0] == 0xE9) {
			original_data.resize(6);
			memmove(original_data.data(), pointer, original_data.size());

			auto* target = follow_branch(data_ptr);
			nop(data_ptr, 1);
			jump(data_ptr + 1, target);
		}
		else if (data_ptr[0] == 0xFF && data_ptr[1] == 0x25) {
			original_data.resize(15);
			memmove(original_data.data(), pointer, original_data.size());

			copy(data_ptr + 1, data_ptr, 14);
			nop(data_ptr, 1);
		}
		else {
			throw std::runtime_error("No branch instruction found");
		}

		return original_data;
	}

	std::vector<std::uint8_t> move_hook(const std::size_t pointer) {
		return move_hook(reinterpret_cast<void*>(pointer));
	}

	void* follow_branch(void* address) {
		auto* const data = static_cast<std::uint8_t*>(address);
		if (*data != 0xE8 && *data != 0xE9) {
			throw std::runtime_error("No branch instruction found");
		}

		return extract<void*>(data + 1);
	}
}

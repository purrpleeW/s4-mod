#pragma once
#include "common_core.hpp"

namespace utils::nt {
	class library_identifier final {
	public:
		class um_hash {
		public:
			std::size_t operator()(const library_identifier& k) const noexcept {
				std::size_t h1 = std::hash<uint64_t>{}(k.base_);
				std::size_t h2 = std::hash<uint64_t>{}(k.packed_info_);
				return h1 ^ (h2 << 1);
			}
		};

		class um_equal {
		public:
			bool operator()(const library_identifier& a, const library_identifier& b) const noexcept {
				return a.base_ == b.base_ && a.packed_info_ == b.packed_info_;
			}
		};

		library_identifier(HMODULE mod) {
			this->base_ = PTR_AS(std::uintptr_t, mod);

			auto dos = PTR_AS(IMAGE_DOS_HEADER*, this->base_);
			if (dos->e_magic != IMAGE_DOS_SIGNATURE) {
				this->packed_info_ = 0;
				return;
			}

			auto nt = PTR_AS(IMAGE_NT_HEADERS*, this->base_ + dos->e_lfanew);
			if (nt->Signature != IMAGE_NT_SIGNATURE) {
				this->packed_info_ = 0;
				return;
			}

			this->module_size_ = nt->OptionalHeader.SizeOfImage;
			this->module_timestamp_ = nt->FileHeader.TimeDateStamp;
		}

		std::uintptr_t base_;
		union {
			struct {
				std::uint32_t module_size_;
				std::uint32_t module_timestamp_;
			};
			std::uint64_t packed_info_;
		};
	};

	class library_page_range final {
	public:
		library_page_range()
			: begin_(0), end_(0), protect_(0), state_(0)
		{}

		library_page_range(std::uintptr_t address)
			: library_page_range(query(address))
		{}

#		if defined(_WIN64)
			library_page_range(MEMORY_BASIC_INFORMATION64 page_info) {
				this->begin_ = page_info.BaseAddress;
				this->end_ = this->begin_ + page_info.RegionSize;
				this->protect_ = page_info.Protect;
				this->state_ = page_info.State;
			}
#		else
			library_page_range(MEMORY_BASIC_INFORMATION page_info) {
				this->begin_ = PTR_AS(std::uintptr_t, page_info.BaseAddress);
				this->end_ = this->begin_ + page_info.RegionSize;
				this->protect_ = page_info.Protect;
				this->state_ = page_info.State;
			}
#		endif

#		if defined(_WIN64)
			static inline MEMORY_BASIC_INFORMATION64 query(std::uintptr_t address) {
				MEMORY_BASIC_INFORMATION64 page_info = {};
				VirtualQuery(PTR_AS(LPCVOID, address), PTR_AS(PMEMORY_BASIC_INFORMATION, &page_info), sizeof(page_info));
				return page_info;
			}
#		else
			static inline MEMORY_BASIC_INFORMATION query(std::uintptr_t address) {
				MEMORY_BASIC_INFORMATION page_info = {};
				VirtualQuery(PTR_AS(LPCVOID, address), PTR_AS(PMEMORY_BASIC_INFORMATION, &page_info), sizeof(page_info));
				return page_info;
			}
#		endif

		bool is_in_range(std::uintptr_t address) const {
			return address >= this->begin_ && address < this->end_;
		}

		bool is_valid() const {
			return this->begin_ != 0 && this->end_ != 0;
		}

		library_page_range next();

		std::uintptr_t begin_;
		std::uintptr_t end_;
		std::uint32_t protect_;
		std::uint32_t state_;
	};

	class library final {
	public:
		static library load(const char* name);
		static library load(const std::string& name);
		static library load(const std::filesystem::path& path);
		static library get_by_address(const void* address);

		library();
		explicit library(const std::string& name);
		explicit library(HMODULE handle);

		library(const library& a)
			: module_(a.module_)
		{}

		bool operator!=(const library& obj) const { return !(*this == obj); };
		bool operator==(const library& obj) const;

		operator bool() const;
		operator HMODULE() const;

		void unprotect() const;
		[[nodiscard]] void* get_entry_point() const;
		[[nodiscard]] std::size_t get_relative_entry_point() const;

		[[nodiscard]] bool is_valid() const;
		[[nodiscard]] std::string get_name() const;
		[[nodiscard]] std::filesystem::path get_path() const;
		[[nodiscard]] std::filesystem::path get_folder() const;
		[[nodiscard]] std::uint8_t* get_ptr() const;
		void free();

		[[nodiscard]] HMODULE get_handle() const;

		template <typename T>
		[[nodiscard]] T get_proc(const char* process) const {
			if (!this->is_valid()) {
				return T{};
			}
			return reinterpret_cast<T>(GetProcAddress(this->module_, process));
		}

		template <typename T>
		[[nodiscard]] T get_proc(const std::string& process) const {
			return get_proc<T>(process.data());
		}

		template <typename T>
		[[nodiscard]] T get(const std::string& process) const {
			if (!this->is_valid()) {
				return nullptr;
			}
			return reinterpret_cast<T>(this->get_proc<void*>(process));
		}

		template <typename T, typename... Args>
		T invoke(const std::string& process, Args ... args) const {
			auto method = this->get<T(__cdecl*)(Args ...)>(process);
			if (method) {
				return method(args...);
			}
			return T();
		}

		template <typename T = void, typename... Args>
		T invoke_pascal(const std::string& process, Args ... args) const {
			auto method = this->get<T(__stdcall*)(Args ...)>(process);
			if (method) {
				return method(args...);
			}
			return T();
		}

		template <typename T, typename... Args>
		T invoke_this(const std::string& process, void* this_ptr, Args ... args) const {
			auto method = this->get<T(__thiscall*)(void*, Args ...)>(this_ptr, process);
			if (method) {
				return method(args...);
			}
			return T();
		}

		[[nodiscard]] std::vector<PIMAGE_SECTION_HEADER> get_section_headers() const;

		[[nodiscard]] PIMAGE_NT_HEADERS get_nt_headers() const;
		[[nodiscard]] PIMAGE_DOS_HEADER get_dos_header() const;
		[[nodiscard]] PIMAGE_OPTIONAL_HEADER get_optional_header() const;
		[[nodiscard]] MODULEINFO get_info() const;

		[[nodiscard]] void** get_iat_entry(const std::string& module_name, std::string proc_name) const;
		[[nodiscard]] void** get_iat_entry(const std::string& module_name, const char* proc_name) const;
		[[nodiscard]] std::map<std::string, std::map<std::string, void**>> get_all_iat_entries() const;

		[[nodiscard]] std::vector<library_page_range> get_accessible_pages();

		bool is_address_in_range(std::uintptr_t address) const;
		bool is_address_in_accessible_range(std::uintptr_t address);
		std::uint32_t get_checksum() const;

	private:
		HMODULE module_;
		static inline std::unordered_map<library_identifier, std::vector<library_page_range>,
			library_identifier::um_hash, library_identifier::um_equal> accessible_pages_;
	};

	class handle {
	public:
		handle() = default;

		handle(const HANDLE h)
			: handle_(h)
		{}

		~handle() {
			if (*this) {
				CloseHandle(this->handle_);
				this->handle_ = invalid_;
			}
		}

		handle(const handle&) = delete;
		handle& operator=(const handle&) = delete;

		handle(handle&& obj) noexcept
			: handle()
		{
			this->operator=(std::move(obj));
		}

		handle& operator=(handle&& obj) noexcept {
			if (this != &obj) {
				this->~handle();
				this->handle_ = obj.handle_;
				obj.handle_ = invalid_;
			}

			return *this;
		}

		handle& operator=(HANDLE h) noexcept {
			this->~handle();
			this->handle_ = h;

			return *this;
		}

		operator bool() const {
			return this->handle_ != invalid_;
		}

		operator HANDLE() const {
			return this->handle_;
		}

	private:
		HANDLE handle_{nullptr};
		HANDLE invalid_{PVOID(-1)};
	};

	class registry_key {
	public:
		registry_key() = default;

		registry_key(HKEY key)
			: key_(key)
		{}

		registry_key(const registry_key&) = delete;
		registry_key& operator=(const registry_key&) = delete;

		registry_key(registry_key&& obj) noexcept
			: registry_key()
		{
			this->operator=(std::move(obj));
		}

		registry_key& operator=(registry_key&& obj) noexcept {
			if (this != obj.GetRef()) {
				this->~registry_key();
				this->key_ = obj.key_;
				obj.key_ = nullptr;
			}

			return *this;
		}

		~registry_key() {
			if (this->key_) {
				RegCloseKey(this->key_);
			}
		}

		operator HKEY() const {
			return this->key_;
		}

		operator bool() const {
			return this->key_ != nullptr;
		}

		HKEY* operator&() {
			return &this->key_;
		}

		registry_key* GetRef() {
			return this;
		}

		const registry_key* GetRef() const {
			return this;
		}

	private:
		HKEY key_{};
	};

	registry_key open_or_create_registry_key(const HKEY base, const std::string& input);

	bool is_wine();
	bool is_shutdown_in_progress();

	__declspec(noreturn) void raise_hard_exception();
	std::string load_resource(int id);

	void relaunch_self();
	__declspec(noreturn) void terminate(std::uint32_t code = 0);

	void show_error(const std::string& text, const std::string& title = "Error");

	bool is_nt_10();

	std::string get_user_name();
	std::string virtual_key_to_string(int virtual_key);
}

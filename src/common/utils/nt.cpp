#include "utils/nt.hpp"

#include "utils/string.hpp"
#include "logger/logger.hpp"
#include <basetsd.h>
#include <lmcons.h>
#include <fstream>
#include <string>
#include <unordered_map>

namespace utils::nt {
	library_page_range library_page_range::next() {
		if (!this->is_valid()) {
			return {};
		}
		auto parent_library = library::get_by_address(PTR_AS(void*, this->begin_));
		if (!parent_library.is_valid()) {
			return {};
		}
		auto parent_library_end = PTR_AS(std::uintptr_t, parent_library.get_ptr()) + parent_library.get_info().SizeOfImage;
		if (this->end_ >= parent_library_end) {
			return {};
		}
		return { query(this->end_) };
	}

	library library::load(const char* name) {
		return library(LoadLibraryA(name));
	}

	library library::load(const std::string& name) {
		return library::load(name.c_str());
	}

	library library::load(const std::filesystem::path& path) {
		return library::load(path.generic_string());
	}

	library library::get_by_address(const void* address) {
		HMODULE handle = nullptr;
		GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, static_cast<LPCSTR>(address), &handle);
		return library(handle);
	}

	library::library()
		: module_(GetModuleHandleA(nullptr))
	{}

	library::library(const std::string& name)
		: module_(GetModuleHandleA(name.c_str()))
	{}

	library::library(const HMODULE handle)
		: module_(handle)
	{}

	bool library::operator==(const library& obj) const {
		return this->module_ == obj.module_;
	}

	library::operator bool() const {
		return this->is_valid();
	}

	library::operator HMODULE() const {
		return this->get_handle();
	}

	PIMAGE_NT_HEADERS library::get_nt_headers() const {
		if (!this->is_valid()) {
			return nullptr;
		}
		return reinterpret_cast<PIMAGE_NT_HEADERS>(this->get_ptr() + this->get_dos_header()->e_lfanew);
	}

	PIMAGE_DOS_HEADER library::get_dos_header() const {
		return reinterpret_cast<PIMAGE_DOS_HEADER>(this->get_ptr());
	}

	PIMAGE_OPTIONAL_HEADER library::get_optional_header() const {
		if (!this->is_valid()) {
			return nullptr;
		}
		return &this->get_nt_headers()->OptionalHeader;
	}

	MODULEINFO library::get_info() const {
		MODULEINFO info{};
		if (!this->is_valid()) {
			return info;
		}

		GetModuleInformation(GetCurrentProcess(), this->module_, &info, sizeof(MODULEINFO));
		return info;
	}

	void** library::get_iat_entry(const std::string& module_name, std::string proc_name) const {
		return this->get_iat_entry(module_name, proc_name.data());
	}

	std::vector<PIMAGE_SECTION_HEADER> library::get_section_headers() const {
		std::vector<PIMAGE_SECTION_HEADER> headers;

		auto nt_headers = this->get_nt_headers();
		auto section = IMAGE_FIRST_SECTION(nt_headers);

		for (std::uint16_t i = 0; i < nt_headers->FileHeader.NumberOfSections; ++i, ++section) {
			if (section) {
				headers.push_back(section);
			}
			else {
				OutputDebugStringA("There was an invalid section :O");
			}
		}

		return headers;
	}

	std::uint8_t* library::get_ptr() const {
		return reinterpret_cast<std::uint8_t*>(this->module_);
	}

	void library::unprotect() const {
		if (!this->is_valid()) {
			return;
		}

		DWORD protection;
		VirtualProtect(this->get_ptr(), this->get_optional_header()->SizeOfImage, PAGE_EXECUTE_READWRITE, &protection);
	}

	std::size_t library::get_relative_entry_point() const {
		if (!this->is_valid()) {
			return 0;
		}
		return this->get_nt_headers()->OptionalHeader.AddressOfEntryPoint;
	}

	void* library::get_entry_point() const {
		if (!this->is_valid()) {
			return nullptr;
		}
		return this->get_ptr() + this->get_relative_entry_point();
	}

	bool library::is_valid() const {
		return this->module_ != nullptr && this->get_dos_header()->e_magic == IMAGE_DOS_SIGNATURE;
	}

	std::string library::get_name() const {
		if (!this->is_valid()) {
			return {};
		}

		return this->get_path().filename().generic_string();
	}

	std::filesystem::path library::get_path() const {
		static auto& path_cache = *new std::unordered_map<HMODULE, std::wstring>();
		static auto& cache_mutex = *new std::mutex();

		if (!this->is_valid()) {
			return {};
		}

		std::lock_guard lock(cache_mutex);
		if (auto it = path_cache.find(this->module_); it != path_cache.end()) {
			return it->second;
		}

		wchar_t name[MAX_PATH] = { 0 };
		GetModuleFileNameW(this->module_, name, MAX_PATH);
		path_cache[this->module_] = std::wstring(name);

		return { name };
	}

	std::filesystem::path library::get_folder() const {
		if (!this->is_valid()) {
			return {};
		}

		const auto path = this->get_path();
		return path.parent_path().generic_string();
	}

	void library::free() {
		if (this->is_valid()) {
			FreeLibrary(this->module_);
			this->module_ = nullptr;
		}
	}

	HMODULE library::get_handle() const {
		return this->module_;
	}

	void** library::get_iat_entry(const std::string& module_name, const char* proc_name) const {
		if (!this->is_valid()) {
			return nullptr;
		}

		const library other_module(module_name);
		if (!other_module.is_valid()) {
			return nullptr;
		}

		auto* const target_function = other_module.get_proc<void*>(proc_name);
		if (!target_function) {
			return nullptr;
		}

		auto* header = this->get_optional_header();
		if (!header) {
			return nullptr;
		}

		auto* import_descriptor = PTR_AS(PIMAGE_IMPORT_DESCRIPTOR, this->get_ptr() + header->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

		while (import_descriptor->Name) {
			if (!_stricmp(PTR_AS(char*, this->get_ptr() + import_descriptor->Name), module_name.data())) {
				auto* original_thunk_data = PTR_AS(PIMAGE_THUNK_DATA, import_descriptor->OriginalFirstThunk + this->get_ptr());
				auto* thunk_data = PTR_AS(PIMAGE_THUNK_DATA, import_descriptor->FirstThunk + this->get_ptr());

				while (original_thunk_data->u1.AddressOfData) {
					if (thunk_data->u1.Function == PTR_AS(std::uint64_t, target_function)) {
						return PTR_AS(void**, &thunk_data->u1.Function);
					}

					const std::size_t ordinal_number = original_thunk_data->u1.AddressOfData & 0xFFFFFFF;

					if (ordinal_number <= 0xFFFF) {
						auto* proc = GetProcAddress(other_module.module_, PTR_AS(char*, ordinal_number));
						if (PTR_AS(void*, proc) == target_function) {
							return PTR_AS(void**, &thunk_data->u1.Function);
						}
					}

					++original_thunk_data;
					++thunk_data;
				}
			}

			++import_descriptor;
		}

		return nullptr;
	}

	std::map<std::string, std::map<std::string, void**>> library::get_all_iat_entries() const {
		std::map<std::string, std::map<std::string, void**>> entries{};

		if (!this->is_valid()) {
			return entries;
		}

		auto* header = this->get_optional_header();
		if (!header) {
			return entries;
		}

		auto import_rva = header->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
		if (!import_rva) {
			return entries;
		}

		auto* import_descriptor = PTR_AS(PIMAGE_IMPORT_DESCRIPTOR, this->get_ptr() + import_rva);

		while (import_descriptor->Name) {
			const char* dll_name = PTR_AS(const char*, this->get_ptr() + import_descriptor->Name);
			auto* original_thunk_data = PTR_AS(PIMAGE_THUNK_DATA, import_descriptor->OriginalFirstThunk + this->get_ptr());
			auto* thunk_data = PTR_AS(PIMAGE_THUNK_DATA, import_descriptor->FirstThunk + this->get_ptr());

			auto& dll_map = entries[dll_name];
			while (original_thunk_data->u1.AddressOfData) {
				std::string func_name;
				if (original_thunk_data->u1.Ordinal & IMAGE_ORDINAL_FLAG) {
					WORD ordinal = IMAGE_ORDINAL(original_thunk_data->u1.Ordinal);
					func_name = std::format("#{:d}", ordinal);
				}
				else {
					auto* import_by_name = PTR_AS(PIMAGE_IMPORT_BY_NAME, this->get_ptr() + original_thunk_data->u1.AddressOfData);
					func_name = import_by_name->Name;
				}

				dll_map[func_name] = PTR_AS(void**, &thunk_data->u1.Function);

				++original_thunk_data;
				++thunk_data;
			}

			++import_descriptor;
		}

		return entries;
	}

	std::vector<library_page_range> library::get_accessible_pages() {
		static std::mutex page_mutex{};
		std::lock_guard<std::mutex> lock(page_mutex);

		auto& pages = accessible_pages_[library_identifier(this->module_)];
		if (pages.empty()) {
			auto page = library_page_range(PTR_AS(std::uintptr_t, this->get_ptr()));
			while (page.is_valid()) {
				if (page.protect_ != PAGE_NOACCESS && page.state_ == MEM_COMMIT && !(page.protect_ & PAGE_GUARD)) {
					pages.push_back(page);
				}
				page = page.next();
			}
		}

		return pages;
	}

	bool library::is_address_in_range(std::uintptr_t address) const {
		if (!this->module_) {
			return false;
		}

		return address >= PTR_AS(std::uintptr_t, this->get_ptr())
			&& address <= PTR_AS(std::uintptr_t, this->get_ptr()) + this->get_optional_header()->SizeOfImage;
	}

	bool library::is_address_in_accessible_range(std::uintptr_t address) {
		if (!this->is_address_in_range(address)) {
			return false;
		}

		for (auto& page : this->get_accessible_pages()) {
			if (!page.is_in_range(address)) {
				continue;
			}
			return true;
		}

		return false;
	}

	std::uint32_t library::get_checksum() const {
		std::ifstream file(this->get_path(), std::ios::binary);
		if (!file.is_open()) {
			return 0;
		}

		std::uint32_t checksum = 0;
		char byte;
		while (file.get(byte)) {
			checksum += static_cast<std::uint8_t>(byte);
		}

		return checksum;
	}

	registry_key open_or_create_registry_key(const HKEY base, const std::string& input) {
		const auto parts = string::split(input, '\\');

		registry_key current_key = base;

		for (const auto& part : parts) {
			registry_key new_key{};
			if (RegOpenKeyExA(current_key, part.data(), 0, KEY_ALL_ACCESS, &new_key) == ERROR_SUCCESS) {
				current_key = std::move(new_key);
				continue;
			}

			if (RegCreateKeyExA(current_key, part.data(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, nullptr, &new_key, nullptr) != ERROR_SUCCESS) {
				return {};
			}

			current_key = std::move(new_key);
		}

		return current_key;
	}

	bool is_wine() {
		static const auto has_wine_export = []() -> bool {
			const library ntdll("ntdll.dll");
			return ntdll.get_proc<void*>("wine_get_version");
		}();

		return has_wine_export;
	}

	bool is_shutdown_in_progress() {
		static auto* shutdown_in_progress = [] {
			const library ntdll("ntdll.dll");
			return ntdll.get_proc<BOOLEAN(*)()>("RtlDllShutdownInProgress");
		}();

		return shutdown_in_progress();
	}

	void raise_hard_exception() {
		int data = false;
		const library ntdll("ntdll.dll");
		ntdll.invoke_pascal("RtlAdjustPrivilege", 19, true, false, &data);
		ntdll.invoke_pascal("NtRaiseHardError", 0xC000007B, 0, nullptr, nullptr, 6, &data);
		_Exit(0);
	}

	std::string load_resource(const int id) {
		const auto lib = library::get_by_address(load_resource);
		auto* const res = FindResource(lib, MAKEINTRESOURCE(id), RT_RCDATA);
		if (!res) {
			return {};
		}

		auto* const handle = LoadResource(lib, res);
		if (!handle) {
			return {};
		}

		return std::string(LPSTR(LockResource(handle)), SizeofResource(lib, res));
	}

	void relaunch_self() {
		const auto self = utils::nt::library::get_by_address(relaunch_self);

		STARTUPINFOA startup_info;
		PROCESS_INFORMATION process_info;

		ZeroMemory(&startup_info, sizeof(startup_info));
		ZeroMemory(&process_info, sizeof(process_info));
		startup_info.cb = sizeof(startup_info);

		char current_dir[MAX_PATH];
		GetCurrentDirectoryA(sizeof(current_dir), current_dir);
		auto* const command_line = GetCommandLineA();

		CreateProcessA(self.get_path().generic_string().data(), command_line, nullptr, nullptr, false, NULL, nullptr, current_dir, &startup_info, &process_info);

		if (process_info.hThread && process_info.hThread != INVALID_HANDLE_VALUE) {
			CloseHandle(process_info.hThread);
		}
		if (process_info.hProcess && process_info.hProcess != INVALID_HANDLE_VALUE) {
			CloseHandle(process_info.hProcess);
		}
	}

	void terminate(const std::uint32_t code) {
		TerminateProcess(GetCurrentProcess(), code);
		_Exit(code);
	}

	void show_error(const std::string& text, const std::string& title) {
		MessageBoxA(nullptr, text.data(), title.data(), MB_ICONERROR | MB_SETFOREGROUND | MB_TOPMOST);
	}

	bool is_nt_10() {
		static auto ntdll = library{ "ntdll.dll" };
		OSVERSIONINFOW ver;
		auto status = ntdll.invoke<NTSTATUS>("RtlGetVersion", &ver);
		return status == S_OK && ver.dwMajorVersion >= 10;
	}

	std::string get_user_name() {
		char username[UNLEN + 1];
		DWORD username_len = UNLEN + 1;
		if (!GetUserNameA(username, &username_len)) {
			return {};
		}

		return std::string(username, username_len - 1);
	}

	std::string virtual_key_to_string(int virtual_key) {
		std::uint32_t scan_code = MapVirtualKey(static_cast<std::uint32_t>(virtual_key), MAPVK_VK_TO_VSC);

		// Special handling for some keys (e.g., right-hand keys)
		switch (virtual_key) {
		case VK_LEFT: case VK_UP: case VK_RIGHT: case VK_DOWN:	// Arrow keys
		case VK_PRIOR: case VK_NEXT:							// PgUp, PgDn
		case VK_END: case VK_HOME:
		case VK_INSERT: case VK_DELETE:
		case VK_DIVIDE:											// NumPad /
		case VK_NUMLOCK:
		case VK_RCONTROL: case VK_RMENU:						// Right Ctrl/Alt
			scan_code |= 0x100;
			break;
		}

		char name[128] = { 0 };
		if (GetKeyNameTextA((LONG)(scan_code << 16), name, sizeof(name)) > 0) {
			return std::string(name);
		}

		return "Unknown";
	}
}

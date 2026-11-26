#pragma once
#include "common_core.hpp"
#include "exception/filter.hpp"
#include "logger/logger.hpp"
#include "utils/nt.hpp"

#if defined(_WIN64)
#	define PROXY_API
#	define PROXY_EXPORT(name, x86_arg_size, ordinal) "/EXPORT:" #name "=" #name "_ZP,@" #ordinal
#else
#	define PROXY_API __stdcall
#	define PROXY_EXPORT(name, x86_arg_size, ordinal) "/EXPORT:" #name "=_" #name "_ZP@" #x86_arg_size ",@" #ordinal
#endif

namespace proxy {
	template <typename T>
	T get_export(const std::string& lib_name, const std::string& export_name) {
		char dir[MAX_PATH]{ 0 };
#		if defined(_WIN64)
			GetSystemDirectoryA(dir, sizeof(dir));
#		else
			GetSystemWow64DirectoryA(dir, sizeof(dir));
#		endif

		const auto lib = utils::nt::library::load(dir + "/"s + lib_name);
		return PTR_AS(T, GetProcAddress(lib, export_name.c_str()));
	}

	template <typename T>
	T get_local_export(const std::string& lib_name, const std::string& export_name) {
		const auto lib = utils::nt::library::load(std::filesystem::current_path() / lib_name);
		return PTR_AS(T, GetProcAddress(lib, export_name.c_str()));
	}

	void on_dll_process_attach(HMODULE h_mod, bool server, bool avoid_alloc_console = false);
}

extern "C" {
	[[gnu::dllexport]] inline void zp_axn_print(const std::string& msg) {
		LOG("Arxan", INFO, msg);
	}

	[[gnu::dllexport]] inline bool zp_open_external_crash_handler(EXCEPTION_POINTERS* inf) {
		return exception::launch_external_crash_handler(exception::generate_exception_json(inf));
	}
}

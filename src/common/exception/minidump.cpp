#include "exception/minidump.hpp"
#include "identification/utility.hpp"
#include "utils/compression.hpp"

#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")

namespace exception {
	namespace {
		constexpr MINIDUMP_TYPE get_minidump_type() {
			constexpr auto type = MiniDumpIgnoreInaccessibleMemory //
				| MiniDumpWithHandleData //
				| MiniDumpScanMemory //
				| MiniDumpWithProcessThreadData //
				| MiniDumpWithFullMemoryInfo //
				| MiniDumpWithThreadInfo //
				| MiniDumpWithUnloadedModules;

			return static_cast<MINIDUMP_TYPE>(type);
		}

		std::string get_temp_filename() {
			char filename[MAX_PATH] = { 0 };
			char pathname[MAX_PATH] = { 0 };

			GetTempPathA(sizeof(pathname), pathname);
			GetTempFileNameA(pathname, std::format("{}-", identification::utility::get_client_name()).c_str(), 0, filename);
			return filename;
		}

		std::string get_timestamp() {
			tm ltime{};
			char timestamp[MAX_PATH] = { 0 };
			const auto time = _time64(nullptr);

			_localtime64_s(&ltime, &time);
			strftime(timestamp, sizeof(timestamp) - 1, "%Y-%m-%d-%H-%M-%S", &ltime);

			return timestamp;
		}

		std::string generate_crash_info(const LPEXCEPTION_POINTERS exception_info) {
			std::string info{};
			const auto line = [&info](const std::string& text) {
				info.append(text);
				info.append("\r\n");
			};

			line(std::format("{} Crash Dump", identification::utility::get_client_name()));
			line("");
			// line("Version: "s + VERSION);
			// line("Environment: "s + (game::environment::is_dedi() ? "Dedicated" : "Base"));
			line("Timestamp: "s + get_timestamp());
			line(std::format("Exception: 0x{:08X}", exception_info->ExceptionRecord->ExceptionCode));
			line(std::format("Address: {}", and_rel(PTR_AS(std::uintptr_t, exception_info->ExceptionRecord->ExceptionAddress))));

#			pragma warning(push)
#			pragma warning(disable: 4996)
			OSVERSIONINFOEXA version_info;
			ZeroMemory(&version_info, sizeof(version_info));
			version_info.dwOSVersionInfoSize = sizeof(version_info);
			GetVersionExA(reinterpret_cast<LPOSVERSIONINFOA>(&version_info));
#			pragma warning(pop)

			line(std::format("OS Version: {}.{}", version_info.dwMajorVersion, version_info.dwMinorVersion));
			return info;
		}

		HANDLE write_dump_to_temp_file(const LPEXCEPTION_POINTERS exception_info) {
			MINIDUMP_EXCEPTION_INFORMATION minidump_exception_info = { GetCurrentThreadId(), exception_info, FALSE };

			auto* const file_handle = CreateFileA(get_temp_filename().data(), GENERIC_WRITE | GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS,
				FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr);

			if (!MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file_handle, get_minidump_type(), &minidump_exception_info, nullptr, nullptr)) {
				MessageBoxA(nullptr, "There was an error creating the minidump! Hit OK to close the program.", "Minidump Error", MB_OK | MB_ICONERROR);
				TerminateProcess(GetCurrentProcess(), 123);
			}

			return file_handle;
		}

		std::string read_file(const HANDLE file_handle) {
			FlushFileBuffers(file_handle);
			SetFilePointer(file_handle, 0, nullptr, FILE_BEGIN);

			std::string buffer{};

			DWORD bytes_read = 0;
			char temp_bytes[0x2000];

			do {
				if (!ReadFile(file_handle, temp_bytes, sizeof(temp_bytes), &bytes_read, nullptr)) {
					return {};
				}

				buffer.append(temp_bytes, bytes_read);
			}
			while (bytes_read == sizeof(temp_bytes));

			return buffer;
		}
	}

	std::string create_minidump(const LPEXCEPTION_POINTERS exception_info) {
		const utils::nt::handle file_handle = write_dump_to_temp_file(exception_info);
		return read_file(file_handle);
	}

	void write_minidump(const LPEXCEPTION_POINTERS exception_info) {
		auto crash_name = std::format("minidumps/{}-crash-{}.zip", identification::utility::get_client_name(), get_timestamp());

		utils::compression::zip::archive zip_file{};
		zip_file.add("crash.dmp", create_minidump(exception_info));
		zip_file.add("info.txt", generate_crash_info(exception_info));
		zip_file.write(crash_name, std::format("{} Crash Dump", identification::utility::get_client_name()));
	}
}

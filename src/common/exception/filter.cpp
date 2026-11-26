#include "common_core.hpp"
#include "exception/filter.hpp"
#include "identification/utility.hpp"
#include "identification/game.hpp"
#include "logger/logger.hpp"
#include "utils/cryptography.hpp"
#include "utils/nt.hpp"

#include <mutex>

namespace exception {
	namespace {
		void* init_dbg_help() {
			static class init_dbg_help_ {
			public:
				init_dbg_help_() {
					SymInitialize(GetCurrentProcess(), nullptr, FALSE);
				}

				~init_dbg_help_() {
					SymCleanup(GetCurrentProcess());
				}
			} init_dbg_help;
			return &init_dbg_help;
		}
	}

	void ensure_dbg_help() {
		(void)init_dbg_help();
	}

	nlohmann::json generate_exception_json(EXCEPTION_POINTERS* inf) {
		nlohmann::json crash_handler_data{};

		HMODULE h_mods[1024];
		DWORD cb_needed;
		HANDLE h_process = GetCurrentProcess();
		if (EnumProcessModules(h_process, h_mods, sizeof(h_mods), &cb_needed)) {
			int module_count = cb_needed / sizeof(HMODULE);
			crash_handler_data["loaded_modules"] = {};
			for (int i = 0; i < module_count; ++i) {
				TCHAR sz_mod_name[MAX_PATH];
				MODULEINFO mod_info = {};
				if (GetModuleFileNameEx(h_process, h_mods[i], sz_mod_name, sizeof(sz_mod_name) / sizeof(TCHAR))
					&& GetModuleInformation(h_process, h_mods[i], &mod_info, sizeof(mod_info)))
				{
					crash_handler_data["loaded_modules"][sz_mod_name] = {
						{ "base", PTR_AS(std::int64_t, h_mods[i]) },
						{ "size", mod_info.SizeOfImage }
					};
				}
			}
		}

		auto exr = inf->ExceptionRecord;
		crash_handler_data["exception_record"]["ExceptionCode"] = exr->ExceptionCode;
		crash_handler_data["exception_record"]["ExceptionFlags"] = exr->ExceptionFlags;
		crash_handler_data["exception_record"]["ExceptionAddress"] = PTR_AS(std::int64_t, exr->ExceptionAddress);
		for (auto x = 0; x < exr->NumberParameters; x++) {
			crash_handler_data["exception_record"]["ExceptionInformation"].push_back(static_cast<std::int64_t>(exr->ExceptionInformation[x]));
		}

		auto cxr = inf->ContextRecord;

#		if defined(_WIN64)
			/* Register parameter home addresses */
			crash_handler_data["context_record"]["P1Home"] = static_cast<std::int64_t>(cxr->P1Home);
			crash_handler_data["context_record"]["P2Home"] = static_cast<std::int64_t>(cxr->P2Home);
			crash_handler_data["context_record"]["P3Home"] = static_cast<std::int64_t>(cxr->P3Home);
			crash_handler_data["context_record"]["P4Home"] = static_cast<std::int64_t>(cxr->P4Home);
			crash_handler_data["context_record"]["P5Home"] = static_cast<std::int64_t>(cxr->P5Home);
			crash_handler_data["context_record"]["P6Home"] = static_cast<std::int64_t>(cxr->P6Home);

			/* Control flags */
			crash_handler_data["context_record"]["ContextFlags"] = cxr->ContextFlags;
			crash_handler_data["context_record"]["MxCsr"] = cxr->MxCsr;

			/* Segment Registers and processor flags */
			crash_handler_data["context_record"]["SegCs"] = cxr->SegCs;
			crash_handler_data["context_record"]["SegDs"] = cxr->SegDs;
			crash_handler_data["context_record"]["SegEs"] = cxr->SegEs;
			crash_handler_data["context_record"]["SegFs"] = cxr->SegFs;
			crash_handler_data["context_record"]["SegGs"] = cxr->SegGs;
			crash_handler_data["context_record"]["SegSs"] = cxr->SegSs;
			crash_handler_data["context_record"]["EFlags"] = cxr->EFlags;

			/* Debug registers */
			crash_handler_data["context_record"]["Dr0"] = static_cast<std::int64_t>(cxr->Dr0);
			crash_handler_data["context_record"]["Dr1"] = static_cast<std::int64_t>(cxr->Dr1);
			crash_handler_data["context_record"]["Dr2"] = static_cast<std::int64_t>(cxr->Dr2);
			crash_handler_data["context_record"]["Dr3"] = static_cast<std::int64_t>(cxr->Dr3);
			crash_handler_data["context_record"]["Dr6"] = static_cast<std::int64_t>(cxr->Dr6);
			crash_handler_data["context_record"]["Dr7"] = static_cast<std::int64_t>(cxr->Dr7);

			/* Integer registers */
			crash_handler_data["context_record"]["Rax"] = static_cast<std::int64_t>(cxr->Rax);
			crash_handler_data["context_record"]["Rcx"] = static_cast<std::int64_t>(cxr->Rcx);
			crash_handler_data["context_record"]["Rdx"] = static_cast<std::int64_t>(cxr->Rdx);
			crash_handler_data["context_record"]["Rbx"] = static_cast<std::int64_t>(cxr->Rbx);
			crash_handler_data["context_record"]["Rsp"] = static_cast<std::int64_t>(cxr->Rsp);
			crash_handler_data["context_record"]["Rbp"] = static_cast<std::int64_t>(cxr->Rbp);
			crash_handler_data["context_record"]["Rsi"] = static_cast<std::int64_t>(cxr->Rsi);
			crash_handler_data["context_record"]["Rdi"] = static_cast<std::int64_t>(cxr->Rdi);
			crash_handler_data["context_record"]["R8"] = static_cast<std::int64_t>(cxr->R8);
			crash_handler_data["context_record"]["R9"] = static_cast<std::int64_t>(cxr->R9);
			crash_handler_data["context_record"]["R10"] = static_cast<std::int64_t>(cxr->R10);
			crash_handler_data["context_record"]["R11"] = static_cast<std::int64_t>(cxr->R11);
			crash_handler_data["context_record"]["R12"] = static_cast<std::int64_t>(cxr->R12);
			crash_handler_data["context_record"]["R13"] = static_cast<std::int64_t>(cxr->R13);
			crash_handler_data["context_record"]["R14"] = static_cast<std::int64_t>(cxr->R14);
			crash_handler_data["context_record"]["R15"] = static_cast<std::int64_t>(cxr->R15);

			/* Program counter */
			crash_handler_data["context_record"]["Rip"] = static_cast<std::int64_t>(cxr->Rip);

			/* Vector registers */
			crash_handler_data["context_record"]["VectorControl"] = static_cast<std::int64_t>(cxr->VectorControl);

			/* Special debug control registers */
			crash_handler_data["context_record"]["DebugControl"] = static_cast<std::int64_t>(cxr->DebugControl);
			crash_handler_data["context_record"]["LastBranchToRip"] = static_cast<std::int64_t>(cxr->LastBranchToRip);
			crash_handler_data["context_record"]["LastBranchFromRip"] = static_cast<std::int64_t>(cxr->LastBranchFromRip);
			crash_handler_data["context_record"]["LastExceptionToRip"] = static_cast<std::int64_t>(cxr->LastExceptionToRip);
			crash_handler_data["context_record"]["LastExceptionFromRip"] = static_cast<std::int64_t>(cxr->LastExceptionFromRip);
#		else
			crash_handler_data["context_record"]["ContextFlags"] = cxr->ContextFlags;

			crash_handler_data["context_record"]["Dr0"] = cxr->Dr0;
			crash_handler_data["context_record"]["Dr1"] = cxr->Dr1;
			crash_handler_data["context_record"]["Dr2"] = cxr->Dr2;
			crash_handler_data["context_record"]["Dr3"] = cxr->Dr3;
			crash_handler_data["context_record"]["Dr6"] = cxr->Dr6;
			crash_handler_data["context_record"]["Dr7"] = cxr->Dr7;

			crash_handler_data["context_record"]["SegGs"] = cxr->SegGs;
			crash_handler_data["context_record"]["SegFs"] = cxr->SegFs;
			crash_handler_data["context_record"]["SegEs"] = cxr->SegEs;
			crash_handler_data["context_record"]["SegDs"] = cxr->SegDs;

			crash_handler_data["context_record"]["Edi"] = cxr->Edi;
			crash_handler_data["context_record"]["Esi"] = cxr->Esi;
			crash_handler_data["context_record"]["Ebx"] = cxr->Ebx;
			crash_handler_data["context_record"]["Edx"] = cxr->Edx;
			crash_handler_data["context_record"]["Ecx"] = cxr->Ecx;
			crash_handler_data["context_record"]["Eax"] = cxr->Eax;

			crash_handler_data["context_record"]["Ebp"] = cxr->Ebp;
			crash_handler_data["context_record"]["Eip"] = cxr->Eip;
			crash_handler_data["context_record"]["SegCs"] = cxr->SegCs;
			crash_handler_data["context_record"]["EFlags"] = cxr->EFlags;
			crash_handler_data["context_record"]["Esp"] = cxr->Esp;
			crash_handler_data["context_record"]["SegSs"] = cxr->SegSs;
#		endif

		auto game = identification::game::get_target_game();
		crash_handler_data["game"]["name"] = game.game_name;
		crash_handler_data["game"]["version"] = identification::game::get_version();

		auto callstack = get_callstack(cxr);
		for (auto& address : callstack) {
			crash_handler_data["callstack"].push_back(static_cast<std::int64_t>(address));
		}

		return crash_handler_data;
	}

	bool launch_external_crash_handler(const nlohmann::json& arg) {
		std::string exe_path = "zpCrashHandler.exe";
		std::string command_line = "\"" + exe_path + "\" \"" + utils::cryptography::base64::encode(arg.dump()) + "\"";

		STARTUPINFOA si = {
			sizeof(si)
		};
		PROCESS_INFORMATION pi = {};

		BOOL success = CreateProcessA(nullptr, &command_line[0], nullptr, nullptr, FALSE, CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi);
		if (!success) {
			return false;
		}

		WaitForSingleObject(pi.hProcess, INFINITE);

		DWORD exit_code = 0;
		GetExitCodeProcess(pi.hProcess, &exit_code);
		if (exit_code != 0) {
			return false;
		}

		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		return true;
	}

	std::vector<std::uintptr_t> get_callstack(const CONTEXT* ctx) {
		static std::mutex stack_walk_mutex;
		const std::lock_guard lock(stack_walk_mutex);
		ensure_dbg_help();

		auto unwind_context = *ctx;
		std::vector<std::uintptr_t> addresses{};

		auto current_process = GetCurrentProcess();
		auto current_thread = GetCurrentThread();

#		if defined(_WIN64)
#			define ZP_StackWalk(stack_frame, context_record) StackWalk64(IMAGE_FILE_MACHINE_AMD64, current_process, current_thread, stack_frame, context_record, \
				nullptr, SymFunctionTableAccess64, SymGetModuleBase64, nullptr)

			STACKFRAME64 stack_frame = { };

			stack_frame.AddrPC.Offset = ctx->Rip;
			stack_frame.AddrFrame.Offset = ctx->Rbp;
			stack_frame.AddrStack.Offset = ctx->Rsp;
#		else
#			define ZP_StackWalk(stack_frame, context_record) StackWalk(IMAGE_FILE_MACHINE_I386, current_process, current_thread, stack_frame, context_record, \
				nullptr, SymFunctionTableAccess, SymGetModuleBase, nullptr)

			STACKFRAME stack_frame = { };

			stack_frame.AddrPC.Offset = ctx->Eip;
			stack_frame.AddrFrame.Offset = ctx->Ebp;
			stack_frame.AddrStack.Offset = ctx->Esp;
#		endif

		stack_frame.AddrPC.Mode = AddrModeFlat;
		stack_frame.AddrFrame.Mode = AddrModeFlat;
		stack_frame.AddrStack.Mode = AddrModeFlat;

		while (ZP_StackWalk(&stack_frame, &unwind_context)) {
			addresses.push_back(stack_frame.AddrPC.Offset);
		}

		return addresses;

#		undef ZP_StackWalk
	}

	long WINAPI filter(EXCEPTION_POINTERS* inf) {
		ensure_dbg_help();

		const char* exception_code_name = "<unknown>";
		static std::map<DWORD, std::string> exception_names{
			{ EXCEPTION_ACCESS_VIOLATION, "Access violation" },
			{ EXCEPTION_ARRAY_BOUNDS_EXCEEDED, "Array bounds exceeded" },
			{ EXCEPTION_BREAKPOINT, "Breakpoint" },
			{ EXCEPTION_DATATYPE_MISALIGNMENT, "Data type misalignment" },
			{ EXCEPTION_FLT_DENORMAL_OPERAND, "Float denormal operand" },
			{ EXCEPTION_FLT_DIVIDE_BY_ZERO, "Float / 0" },
			{ EXCEPTION_FLT_INEXACT_RESULT, "Float inexact result" },
			{ EXCEPTION_FLT_INVALID_OPERATION, "Float invalid operation" },
			{ EXCEPTION_FLT_OVERFLOW, "Float overflow" },
			{ EXCEPTION_FLT_STACK_CHECK, "Float stack check" },
			{ EXCEPTION_FLT_UNDERFLOW, "Float underflow" },
			{ EXCEPTION_ILLEGAL_INSTRUCTION, "Illegal instruction" },
			{ EXCEPTION_IN_PAGE_ERROR, "In-page error" },
			{ EXCEPTION_INT_DIVIDE_BY_ZERO, "Int / 0" },
			{ EXCEPTION_INT_OVERFLOW, "Integer overflow" },
			{ EXCEPTION_INVALID_DISPOSITION, "Invalid disposition" },
			{ EXCEPTION_NONCONTINUABLE_EXCEPTION, "Non-continuable exception" },
			{ EXCEPTION_PRIV_INSTRUCTION, "Private instruction" },
			{ EXCEPTION_SINGLE_STEP, "Single step" },
			{ EXCEPTION_STACK_OVERFLOW, "Stack overflow" }
		};
		if (exception_names.contains(inf->ExceptionRecord->ExceptionCode)) {
			exception_code_name = exception_names[inf->ExceptionRecord->ExceptionCode].c_str();
		}

		utils::nt::library game{};
		utils::nt::library client = utils::nt::library(client_module);

		std::string title = std::format("{} exception handler", identification::game::get_target_game().client_name);

		std::string message = "Unhandled exception caught. Instead of giving you absolutely nothing:\n";
		message += std::format("{} region: 0x" FMT_PTR " -> 0x" FMT_PTR "\n", game.get_name(),
			PTR_AS(std::uintptr_t, game.get_ptr()), PTR_AS(std::uintptr_t, game.get_ptr()) + game.get_optional_header()->SizeOfImage);
		message += std::format("{} region: 0x" FMT_PTR " -> 0x" FMT_PTR "\n", client.get_name(),
			PTR_AS(std::uintptr_t, client.get_ptr()), PTR_AS(std::uintptr_t, client.get_ptr()) + client.get_optional_header()->SizeOfImage);
		message += "---- Exception Record ----\n";
		message += std::format("Exception code: {} (0x{:08X})\n", exception_code_name, inf->ExceptionRecord->ExceptionCode);
		message += std::format("Exception flags: 0x{:08X}\n", inf->ExceptionRecord->ExceptionFlags);
		message += std::format("Exception address: {}\n", and_rel(PTR_AS(std::uintptr_t, inf->ExceptionRecord->ExceptionAddress)));
		message += std::format("Number of parameters: {}\n", inf->ExceptionRecord->NumberParameters);
		for (auto x = 0ul; x < std::min(inf->ExceptionRecord->NumberParameters, 15ul /*EXCEPTION_MAXIMUM_PARAMETERS*/); x++) {
			message += std::format(" > Parameter {}: {}\n", x, and_rel(inf->ExceptionRecord->ExceptionInformation[x]));
		}
		message += "----- Context Record -----\n";
#		if defined(_WIN64)
			message += std::format("P1Home: {}\n", and_rel(inf->ContextRecord->P1Home));
			message += std::format("P2Home: {}\n", and_rel(inf->ContextRecord->P2Home));
			message += std::format("P3Home: {}\n", and_rel(inf->ContextRecord->P3Home));
			message += std::format("P4Home: {}\n", and_rel(inf->ContextRecord->P4Home));
			message += std::format("P5Home: {}\n", and_rel(inf->ContextRecord->P5Home));
			message += std::format("P6Home: {}\n", and_rel(inf->ContextRecord->P6Home));

			message += std::format("Context flags: 0x{:08X}\n", inf->ContextRecord->ContextFlags);
			message += std::format("MxCsr: 0x{:08X}\n", inf->ContextRecord->MxCsr);

			message += std::format("SegCs: 0x{:04X}\n", inf->ContextRecord->SegCs);
			message += std::format("SegDs: 0x{:04X}\n", inf->ContextRecord->SegDs);
			message += std::format("SegEs: 0x{:04X}\n", inf->ContextRecord->SegEs);
			message += std::format("SegFs: 0x{:04X}\n", inf->ContextRecord->SegFs);
			message += std::format("SegGs: 0x{:04X}\n", inf->ContextRecord->SegGs);
			message += std::format("SegSs: 0x{:04X}\n", inf->ContextRecord->SegSs);
			message += std::format("EFlags: 0x{:08X}\n", inf->ContextRecord->EFlags);

			message += std::format("Dr0: {}\n", and_rel(inf->ContextRecord->Dr0));
			message += std::format("Dr1: {}\n", and_rel(inf->ContextRecord->Dr1));
			message += std::format("Dr2: {}\n", and_rel(inf->ContextRecord->Dr2));
			message += std::format("Dr3: {}\n", and_rel(inf->ContextRecord->Dr3));
			message += std::format("Dr6: {}\n", and_rel(inf->ContextRecord->Dr6));
			message += std::format("Dr7: {}\n", and_rel(inf->ContextRecord->Dr7));

			message += std::format("Rax: {}\n", and_rel(inf->ContextRecord->Rax));
			message += std::format("Rcx: {}\n", and_rel(inf->ContextRecord->Rcx));
			message += std::format("Rdx: {}\n", and_rel(inf->ContextRecord->Rdx));
			message += std::format("Rbx: {}\n", and_rel(inf->ContextRecord->Rbx));
			message += std::format("Rsp: {}\n", and_rel(inf->ContextRecord->Rsp));
			message += std::format("Rbp: {}\n", and_rel(inf->ContextRecord->Rbp));
			message += std::format("Rsi: {}\n", and_rel(inf->ContextRecord->Rsi));
			message += std::format("Rdi: {}\n", and_rel(inf->ContextRecord->Rdi));
			message += std::format("R8: {}\n", and_rel(inf->ContextRecord->R8));
			message += std::format("R9: {}\n", and_rel(inf->ContextRecord->R9));
			message += std::format("R10: {}\n", and_rel(inf->ContextRecord->R10));
			message += std::format("R11: {}\n", and_rel(inf->ContextRecord->R11));
			message += std::format("R12: {}\n", and_rel(inf->ContextRecord->R12));
			message += std::format("R13: {}\n", and_rel(inf->ContextRecord->R13));
			message += std::format("R14: {}\n", and_rel(inf->ContextRecord->R14));
			message += std::format("R15: {}\n", and_rel(inf->ContextRecord->R15));

			message += std::format("Rip: {}\n", and_rel(inf->ContextRecord->Rip));

			message += std::format("VectorControl: {}\n", and_rel(inf->ContextRecord->VectorControl));

			message += std::format("DebugControl: {}\n", and_rel(inf->ContextRecord->DebugControl));
			message += std::format("LastBranchToRip: {}\n", and_rel(inf->ContextRecord->LastBranchToRip));
			message += std::format("LastBranchFromRip: {}\n", and_rel(inf->ContextRecord->LastBranchFromRip));
			message += std::format("LastExceptionToRip: {}\n", and_rel(inf->ContextRecord->LastExceptionToRip));
			message += std::format("LastExceptionFromRip: {}\n", and_rel(inf->ContextRecord->LastExceptionFromRip));

			message += "------- Other Info -------\n";
			std::size_t function_count = 0;
			for (const auto address : get_callstack(inf->ContextRecord)) {
				function_count++;
				message += std::format("Function address [{}]: {}\n", function_count, and_rel(address));
			}
#		else
			message += std::format("Context flags: 0x{:08X}\n", inf->ContextRecord->ContextFlags);

			message += std::format("Dr0: {}\n", and_rel(inf->ContextRecord->Dr0));
			message += std::format("Dr1: {}\n", and_rel(inf->ContextRecord->Dr1));
			message += std::format("Dr2: {}\n", and_rel(inf->ContextRecord->Dr2));
			message += std::format("Dr3: {}\n", and_rel(inf->ContextRecord->Dr3));
			message += std::format("Dr6: {}\n", and_rel(inf->ContextRecord->Dr6));
			message += std::format("Dr7: {}\n", and_rel(inf->ContextRecord->Dr7));

			message += std::format("SegGs: 0x{:08X}\n", inf->ContextRecord->SegGs);
			message += std::format("SegFs: 0x{:08X}\n", inf->ContextRecord->SegFs);
			message += std::format("SegEs: 0x{:08X}\n", inf->ContextRecord->SegEs);
			message += std::format("SegDs: 0x{:08X}\n", inf->ContextRecord->SegDs);

			message += std::format("Edi: {}\n", and_rel(inf->ContextRecord->Edi));
			message += std::format("Esi: {}\n", and_rel(inf->ContextRecord->Esi));
			message += std::format("Ebx: {}\n", and_rel(inf->ContextRecord->Ebx));
			message += std::format("Edx: {}\n", and_rel(inf->ContextRecord->Edx));
			message += std::format("Ecx: {}\n", and_rel(inf->ContextRecord->Ecx));
			message += std::format("Eax: {}\n", and_rel(inf->ContextRecord->Eax));

			message += std::format("Ebp: {}\n", and_rel(inf->ContextRecord->Ebp));
			message += std::format("Eip: {}\n", and_rel(inf->ContextRecord->Eip));
			message += std::format("SegCs: 0x{:08X}\n", inf->ContextRecord->SegCs);
			message += std::format("EFlags: 0x{:08X}\n", inf->ContextRecord->EFlags);
			message += std::format("Esp: {}\n", and_rel(inf->ContextRecord->Esp));
			message += std::format("SegSs: 0x{:04X}\n", inf->ContextRecord->SegSs);
#		endif

		MessageBoxA(nullptr, message.c_str(), title.c_str(), MB_OK);
		return EXCEPTION_EXECUTE_HANDLER;
	}
}

#pragma once

#include "common_core.hpp"
#include "ci/ci.hpp"
#include "component/arxan/arxan_base.hpp"

#include <exception/filter.hpp>
#include <exception/minidump.hpp>
#include <loader/component_loader.hpp>
#include <utils/binary_resource.hpp>
#include <utils/concurrency.hpp>
#include <utils/hook.hpp>

#include <excpt.h>
#include <types.h>
#include <extern.h>
#include <winnt.h>

namespace exception_filter {
	namespace detail {
		inline utils::concurrency::container<std::size_t> ignorant_threads{};

		inline utils::hook::iat_detour add_vectored_exception_handler_hook;
		inline utils::hook::iat_detour set_unhandled_exception_filter_hook;

		inline std::function<bool()> should_print_exception_infos_fn = {};
		inline std::function<bool()> should_print_ignored_exception_infos_fn = {};

		inline long __stdcall exception_filter_with_patch(EXCEPTION_POINTERS* inf) {
			if (ci::is_testing()) {
				ci::end_test_run(-128 /* Exception Failure */);
				return EXCEPTION_EXECUTE_HANDLER;
			}

			exception::write_minidump(inf);
			if (!exception::launch_external_crash_handler(exception::generate_exception_json(inf))) {
				return exception::filter(inf);
			}

			return EXCEPTION_CONTINUE_SEARCH;
		}

		inline long __stdcall arxan_exception_filter(EXCEPTION_POINTERS* inf) {
			static utils::nt::library game{};

			auto should_print_exception_infos = should_print_exception_infos_fn && should_print_exception_infos_fn();
			auto should_print_ignored_exception_infos = should_print_ignored_exception_infos_fn && should_print_ignored_exception_infos_fn();

			if (should_print_exception_infos && should_print_ignored_exception_infos) {
				LOG("Component/ExceptionFilter", DEBUG, "Exception 0x{:X} from {}", inf->ExceptionRecord->ExceptionCode,
					and_rel(PTR_AS(std::uintptr_t, inf->ExceptionRecord->ExceptionAddress)));
			}

			if (ignorant_threads.copy() > 0) {
				return EXCEPTION_CONTINUE_SEARCH;
			}

			if (should_print_exception_infos && !should_print_ignored_exception_infos) {
				LOG("Component/ExceptionFilter", DEBUG, "Exception 0x{:X} from {}", inf->ExceptionRecord->ExceptionCode,
					and_rel(PTR_AS(std::uintptr_t, inf->ExceptionRecord->ExceptionAddress)));
			}

			if (inf->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT) {
				return EXCEPTION_CONTINUE_EXECUTION;
			}

			if (inf->ExceptionRecord->ExceptionCode == STATUS_INVALID_HANDLE) {
				return EXCEPTION_CONTINUE_EXECUTION;
			}

			if (inf->ExceptionRecord->ExceptionCode == EXCEPTION_SINGLE_STEP) {
				std::uintptr_t exception_addr = PTR_AS(std::uintptr_t, inf->ExceptionRecord->ExceptionAddress);

				if (inf->ContextRecord->Dr6 & (1 << 0)) {
					static int counter = 0;
					counter++;

					LOG("Component/ExceptionFilter", DEBUG, "ArxanPatch: Breakpoint 1 @ {} [#{}], Rax:{}", and_rel(exception_addr), counter,
						and_rel(inf->ContextRecord->Rax));
				}

				if (inf->ContextRecord->Dr6 & (1 << 1)) {
					static int counter = 0;
					counter++;

					LOG("Component/ExceptionFilter", DEBUG, "ArxanPatch: Breakpoint 2 @ {} [#{}], Rax:{}", and_rel(exception_addr), counter,
						and_rel(inf->ContextRecord->Rax));
				}

				if (inf->ContextRecord->Dr6 & (1 << 2)) {
					static int counter = 0;
					counter++;

					LOG("Component/ExceptionFilter", DEBUG, "ArxanPatch: Breakpoint 3 @ {} [#{}], Rax:{}", and_rel(exception_addr), counter,
						and_rel(inf->ContextRecord->Rax));
				}

				if (inf->ContextRecord->Dr6 & (1 << 3)) {
					LOG("Component/ExceptionFilter", DEBUG, "ArxanPatch: we tried to syscall but you took away our technology");
					//if (inf->ContextRecord->Rax != AllocateVirtualMemorySysCall) {
					//	LOG("Component/ExceptionFilter", DEBUG, "ArxanPatch: syscall %llx\n", and_rel(inf->ContextRecord->Rax));
					//	inf->ContextRecord->Rip = (uint64_t)ntdllAsmStubLocation;
					//}
				}

				inf->ContextRecord->EFlags |= 0x10000 /* ResumeFlag */;
				return EXCEPTION_CONTINUE_EXECUTION;
			}

			if (inf->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION || inf->ExceptionRecord->ExceptionCode == 0xC00000FF) {
				static int access_violation_counter = 0;
				static int access_violation_ignore_limit = arxan::is_set_to_load() ? 2 : 0;
				access_violation_counter++;
				if (access_violation_counter >= access_violation_ignore_limit) {
					if (arxan::should_notify_on_exception() && (PTR_AS(std::uintptr_t, inf->ExceptionRecord->ExceptionAddress) & 0xFFFF) != 0x0FFE) {
						if (ci::is_testing()) {
							ci::end_test_run(-128 /* Exception Failure */);
							return EXCEPTION_CONTINUE_EXECUTION;
						}
						LOG("Component/ExceptionFilter", ERROR, "We may potentionally have experienced a catastrophic game exploision (perchance?)");
						return exception_filter_with_patch(inf);
					}
				}
			}

			return EXCEPTION_CONTINUE_SEARCH;
		}

		inline PVOID add_vectored_exception_handler_stub(ULONG first, PVECTORED_EXCEPTION_HANDLER handler) {
			return handler;
		}

		inline LPTOP_LEVEL_EXCEPTION_FILTER set_unhandled_exception_filter_stub(LPTOP_LEVEL_EXCEPTION_FILTER) {
			return set_unhandled_exception_filter_hook.invoke<LPTOP_LEVEL_EXCEPTION_FILTER>(exception_filter_with_patch);
		}
	}

	inline void extract_crash_handler(int resource_id) {
		std::string external_crash_handler_path = "zpCrashHandler.exe";
		utils::binary_resource external_crash_handler_resource(resource_id, external_crash_handler_path);
		auto external_crash_handler_data = external_crash_handler_resource.get_data();

		std::ofstream out(external_crash_handler_path, std::ios::binary);
		if (out) {
			out.write(external_crash_handler_data.data(), external_crash_handler_data.size());
			out.close();
		}
		else {
			LOG("Component/ExceptionFilter", WARN, "Failed to open {} for writing.", external_crash_handler_path);
		}
	}

	inline void init_exception_filter() {
		// if (!arxan::is_set_to_load()) {
			// SetUnhandledExceptionFilter(detail::exception_filter_with_patch);
			// detail::set_unhandled_exception_filter_hook.create("kernel32.dll", "SetUnhandledExceptionFilter", detail::set_unhandled_exception_filter_stub);
			// return;
		// }

		AddVectoredExceptionHandler(TRUE, detail::arxan_exception_filter);
		/*
		SetUnhandledExceptionFilter(arxan::is_set_to_load() ? detail::arxan_exception_filter : detail::exception_filter_with_patch);
		detail::add_vectored_exception_handler_hook.create("kernel32.dll", "AddVectoredExceptionHandler", detail::add_vectored_exception_handler_stub);
		detail::set_unhandled_exception_filter_hook.create("kernel32.dll", "SetUnhandledExceptionFilter", detail::set_unhandled_exception_filter_stub);
		*/
		/*
		SetUnhandledExceptionFilter(detail::exception_filter_with_patch);
		detail::set_unhandled_exception_filter_hook.create("kernel32.dll", "SetUnhandledExceptionFilter", detail::set_unhandled_exception_filter_stub);
		*/
	}

	inline void run_try_except_internal(const std::function<void()>& try_block, const std::function<void()>& except_block = {}) {
		detail::ignorant_threads.access([&](std::size_t& value) {
			value++;
		});
		__try {
			if (try_block) {
				try_block();
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {
			if (except_block) {
				except_block();
			}
		}
		detail::ignorant_threads.access([&](std::size_t& value) {
			value--;
		});
	}
}
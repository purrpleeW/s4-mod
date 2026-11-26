#include "common.hpp"

#include "component/arxan.hpp"

#include <exception/filter.hpp>
#include <loader/component_loader.hpp>
#include <utils/binary_resource.hpp>
#include <utils/hook.hpp>

#include "resource.h"

namespace exception_filter {
	namespace {
		utils::hook::iat_detour set_unhandled_exception_filter_hook;
		HANDLE veh_handle = nullptr;

		long __stdcall exception_filter_with_patch(EXCEPTION_POINTERS* inf) {
			if (!exception::launch_external_crash_handler(exception::generate_exception_json(inf))) {
				return exception::filter(inf);
			}

			return EXCEPTION_CONTINUE_SEARCH;
		}

		long __stdcall arxan_exception_filter(EXCEPTION_POINTERS* inf) {
			static utils::nt::library game{};

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

			if (inf->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION) {
				static int access_violation_counter = 0;
				access_violation_counter++;
				if (access_violation_counter > 1) {
					if (arxan::should_notify_on_exception && (PTR_AS(std::uintptr_t, inf->ExceptionRecord->ExceptionAddress) & 0xFFFF) != 0x0FFE) {
						LOG("Component/ExceptionFilter", DEBUG, "ArxanPatch: we may or may not have exploded");
						return exception_filter_with_patch(inf);
					}
					else {
						LOG("Component/ExceptionFilter", DEBUG, "ArxanPatch: ignored access violation");
					}
				}
			}

			return EXCEPTION_CONTINUE_SEARCH;
		}
	}

	struct component final : generic_component {
		component_priority priority() const override {
			return component_priority::exception_filter;
		}

		void post_load() override {
			std::string external_crash_handler_path = "zpCrashHandler.exe";
			utils::binary_resource external_crash_handler_resource(EXTERNAL_CRASH_HANDLER, external_crash_handler_path);
			auto external_crash_handler_data = external_crash_handler_resource.get_data();

			std::ofstream out(external_crash_handler_path, std::ios::binary);
			if (out) {
				out.write(external_crash_handler_data.data(), external_crash_handler_data.size());
				out.close();
			}
			else {
				LOG("Component/ExceptionFilter", WARN, "Failed to open {} for writing.", external_crash_handler_path);
			}

			veh_handle = AddVectoredExceptionHandler(TRUE, arxan_exception_filter);
		}

		void pre_destroy() override {
			RemoveVectoredExceptionHandler(veh_handle);
		}
	};
}

REGISTER_COMPONENT(exception_filter::component)

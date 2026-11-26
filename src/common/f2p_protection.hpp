#pragma once

#include "common_core.hpp"
#include "protection.hpp"

#include <utils/hook.hpp>

#if defined(CLIENT_F2P) && CLIENT_F2P
	[[clang::always_inline]] inline void crash_timebomb() {
		CV_VM_PUMA_START;
		// constexpr auto expiry_time_unix = 1773896400ull /* Thu Mar 19 2026 05:00:00 GMT+0000 */;
		constexpr auto expiry_time_unix = 1774857600ull /* Tue Mar 24 2026 05:00:00 GMT+0000 */;
		auto time = std::chrono::system_clock::now();
		auto expiry_tp = std::chrono::time_point<std::chrono::system_clock>(
			std::chrono::seconds(expiry_time_unix)
		);

		if (time > expiry_tp) {
			auto [stub, _] = utils::hook::assemble([](utils::hook::assembler& a) {
				a.push(rbp);
				a.mov(rbp, rsp);
				a.mov(rax, 0x002c); // NtTerminateProcess x64 Windows
				a.syscall();
				a.pop(rbp);
				a.ret();
			});

			typedef void (*NtTerminateProcessFunc)(HANDLE ProcessHandle, UINT ExitStatus);
			NtTerminateProcessFunc terminateFunc = (NtTerminateProcessFunc)stub;
			terminateFunc((HANDLE)-1, 0);
		}
		CV_VM_PUMA_END;
	}

	[[gnu::constructor]] inline void pre_main_timebomb() {
		return crash_timebomb();
	}
#else
	inline void crash_timebomb() {}
#endif

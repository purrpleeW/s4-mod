#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/HostInfo.hpp"

namespace t9 {
	class SessionHost {
	public:
		HostInfo info_;			// 0x0000
		int max_clients_;		// 0x0108
	private:
		char pad_010C[0x0004];	// 0x010C
	};
	ENGINE_ASSERT_SZ(SessionHost, 0x0110);
}

#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/netadr_t.hpp"

namespace iw8 {
	class ping_t {
	public:
		netadr_t adr_;		// 0x0000
		int start_;			// 0x0014
		int time_;			// 0x0018
		char info_[0x400];	// 0x001C
	};
	ENGINE_ASSERT_SZ(ping_t, 0x041C);
}

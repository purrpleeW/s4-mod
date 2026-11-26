#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class NetPingInfo {
	public:
		bool initialized_;			// 0x0000
	private:
		char pad_0001[0x0007];		// 0x0001
	public:
		std::uint64_t net_ticks_;	// 0x0008
		std::uint64_t sock_ticks_;	// 0x0010
	};
	ENGINE_ASSERT_SZ(NetPingInfo, 0x0018);
}

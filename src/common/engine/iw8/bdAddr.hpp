#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/bdRelayRoute.hpp"
#include "engine/iw8/bdSockAddr.hpp"

namespace iw8 {
	class bdAddr {
	public:
		enum class bdAddrType {
			BD_ADDR_DIRECT = 0,
			BD_ADDR_RELAYED = 1
		};

		bdSockAddr address_;		// 0x0000
		bdRelayRoute relay_route_;	// 0x0080
		bdAddrType type_;			// 0x0090
	private:
		char pad_0094[0x0004];		// 0x0094
	};
	ENGINE_ASSERT_SZ(bdAddr, 0x0098);
}

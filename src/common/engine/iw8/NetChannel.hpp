#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/enums/netsrc_t.hpp"

namespace iw8 {
	class NetChannel {
		enum class State {
			STATE_CLOSED = 0,
			STATE_CLOSING = 1,
			STATE_OPEN = 2
		};

		State state_;					// 0x0000
		int reference_count_;			// 0x0004
		NetEndpoint* endpoint_;			// 0x0008
		netsrc_t local_id_;				// 0x0010
		netsrc_t remote_id_;			// 0x0014
	};
	ENGINE_ASSERT_SZ(NetChannel, 0x0018);
}

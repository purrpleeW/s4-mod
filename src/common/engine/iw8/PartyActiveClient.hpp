#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/enums/LocalClientNum_t.hpp"

namespace iw8 {
	class PartyActiveClient {
	public:
		LocalClientNum_t local_client_num_;	// 0x0000
		int local_controller_index_;		// 0x0004
	};
	ENGINE_ASSERT_SZ(PartyActiveClient, 0x0008);
}

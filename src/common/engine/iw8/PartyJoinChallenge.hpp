#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class PartyJoinChallenge {
	public:
		char str_[6];	// 0x0000
	};
	ENGINE_ASSERT_SZ(PartyJoinChallenge, 0x0006);
}

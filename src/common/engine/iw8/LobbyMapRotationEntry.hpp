#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class LobbyMapRotationEntry {
	public:
		char name_[24];
		int weight_;
	};
	ENGINE_ASSERT_SZ(LobbyMapRotationEntry,0x1C);
}

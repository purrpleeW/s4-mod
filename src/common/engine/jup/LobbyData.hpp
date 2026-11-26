#pragma once
#include "engine/engine_common.hpp"
#include "engine/jup/PartyData.hpp"

namespace jup {
	class LobbyData {
	public:
		PartyData party_;
	};
	ENGINE_ASSERT_SZ(LobbyData, sizeof(PartyData));
}

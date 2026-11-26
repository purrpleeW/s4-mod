#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw9/PartyData.hpp"

namespace iw9 {
	class LobbyData {
	public:
		PartyData party_;
	};
	ENGINE_ASSERT_SZ(LobbyData, sizeof(PartyData));
}

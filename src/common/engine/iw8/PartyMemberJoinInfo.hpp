#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/XUID.hpp"
#include "engine/iw8/enums/PartyJoinResponse.hpp"

namespace iw8 {
	class PartyMemberJoinInfo {
	public:
		int active_;							// 0x0000
	private:
		char pad_0004[0x0004];					// 0x0004
	public:
		XUID xuid_;								// 0x0008
		PartyJoinResponse response_state_[2];	// 0x0010
		std::uint32_t responce_message_handle_;	// 0x0018
	private:
		char pad_001C[0x0004];					// 0x001C
	};
	ENGINE_ASSERT_SZ(PartyMemberJoinInfo, 0x0020);
}

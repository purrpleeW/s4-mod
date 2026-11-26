#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/XUID.hpp"
#include "engine/iw8/enums/PresencePlatform.hpp"

namespace iw8 {
	class UIPlayerInfo {
	public:
		char presence_string_[0x80];			// 0x0000
		char last_met_info_[0x80];				// 0x0080
		bool is_session_private_;				// 0x0100
		char gamertag_[0x80];					// 0x0101
		char gamertag_with_hash_[0x40];			// 0x0181
		PresencePlatform presence_platform_;	// 0x01C1
	private:
		char pad_01C2[0x0006];					// 0x01C2
	public:
		XUID xuid_;								// 0x01C8
		std::uint64_t platform_id_;				// 0x01D0
	};
	ENGINE_ASSERT_SZ(UIPlayerInfo, 0x01D8);
}

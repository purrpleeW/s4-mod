#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/CrossTitlePresenceData.hpp"
#include "engine/iw8/enums/PresenceLocation.hpp"

namespace iw8 {
	class SocialPresence {
	public:
		CrossTitlePresenceData cross_title_presence_data_; 		// 0x0000
		char game_type_;										// 0x0012
		char map_index_;										// 0x0013
		PresenceLocation location_;								// 0x0014
	private:
		char pad_0015[11];										// 0x0015
	public:
		unsigned int session_flags_;							// 0x0020
		char clan_tag_[12];										// 0x0024
		char version_;											// 0x0030
		char flags_;											// 0x0031
		char input_type_;										// 0x0032
	private:
		char pad_0033[5];										// 0x0033
	};
	ENGINE_ASSERT_SZ(SocialPresence, 0x0038);
}

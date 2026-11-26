#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/netadr_t.hpp"
#include "engine/iw8/XNADDR.hpp"
#include "engine/iw8/XSECURITY_INFO.hpp"

namespace iw8 {
	class ClServerInfo {
	public:
		XNADDR xn_addr_;				// 0x0000
		XSECURITY_INFO xn_info_;		// 0x0054
		netadr_t adr_;					// 0x006C
		std::uint8_t clients_;			// 0x0080
		std::uint8_t max_clients_;		// 0x0081
		std::uint8_t dirty_;			// 0x0082
		std::uint8_t request_count_;	// 0x0083
		std::int16_t min_ping_;			// 0x0084
		std::int16_t max_ping_;			// 0x0086
		std::int16_t ping_;				// 0x0088
		bool is_dedicated_;				// 0x008A
		std::uint8_t is_lobby_;			// 0x008B
		char host_name_[36];			// 0x008C
		char map_name_[36];				// 0x00B0
		char game_type_[16];			// 0x00D4
	};
	ENGINE_ASSERT_SZ(ClServerInfo, 0x00E4);
}

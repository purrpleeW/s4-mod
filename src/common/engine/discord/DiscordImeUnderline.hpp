#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	class DiscordImeUnderline {
	public:
		int from_;							// 0x0000
		int to_;							// 0x0004
		std::uint32_t color_;				// 0x0008
		std::uint32_t background_color_;	// 0x000C
		bool thick_;						// 0x0010
	private:
		char pad_0011[0x0003];				// 0x0011
	};
	ENGINE_ASSERT_SZ(DiscordImeUnderline, 0x0014);
}

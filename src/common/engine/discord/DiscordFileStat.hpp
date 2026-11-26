#pragma once
#include "engine/engine_common.hpp"

namespace discord {
	class DiscordFileStat {
	public:
		char filename_[260];			// 0x0000
	private:
		char pad_0104[0x0004];			// 0x0104
	public:
		std::uint64_t size_;			// 0x0108
		std::uint64_t last_modified_;	// 0x0110
	};
	ENGINE_ASSERT_SZ(DiscordFileStat, 0x0118);
}

#pragma once
#include "engine/engine_common.hpp"
#include "engine/discord/enums/EDiscordImageType.hpp"

namespace discord {
	class DiscordImageHandle {
	public:
		EDiscordImageType type_;	// 0x0000
	private:
		char pad_0004[0x0004];		// 0x0004
	public:
		std::int64_t id_;			// 0x0008
		std::uint32_t size_;		// 0x0010
	private:
		char pad_0014[0x0004];		// 0x0014
	};
	ENGINE_ASSERT_SZ(DiscordImageHandle, 0x0018);
}

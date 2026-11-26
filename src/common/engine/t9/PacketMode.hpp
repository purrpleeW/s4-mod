#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/enums/PacketModeList.hpp"

namespace t9 {
	class PacketMode {
	public:
		std::uint32_t start_;	// 0x0000
		PacketModeList mode_;	// 0x0004
	};
	ENGINE_ASSERT_SZ(PacketMode, 0x0008);
}

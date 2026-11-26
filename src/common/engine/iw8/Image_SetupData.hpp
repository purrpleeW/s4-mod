#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class Image_SetupData {
	public:
		const std::uint8_t* data[0x0010][0x0600];	// 0x00000000
	};
	ENGINE_ASSERT_SZ(Image_SetupData, 0x00030000);
}

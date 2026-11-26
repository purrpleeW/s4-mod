#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class CmdText {
	public:
		std::uint8_t* data_;	// 0x0000
		int max_size_;			// 0x0008
		int cmd_size_;			// 0x000C
	};
	ENGINE_ASSERT_SZ(CmdText, 0x0010);
}

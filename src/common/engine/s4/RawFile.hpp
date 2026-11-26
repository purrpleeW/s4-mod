#pragma once
#include "engine/engine_common.hpp"

namespace s4 {
	class RawFile {
	public:
		const char* name_;				// 0x0000
		std::uint32_t compressed_len_;	// 0x0008
		std::uint32_t len_;				// 0x000C
		const char* buffer_;			// 0x0010
	};
	ENGINE_ASSERT_SZ(RawFile, 0x0018);
}

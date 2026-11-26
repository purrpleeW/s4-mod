#pragma once
#include "engine/engine_common.hpp"

namespace sat {
	class RawFile {
	public:
		XHash hash_;					// 0x0000
	private:
		char pad_0008[0x0004];			// 0x0008
	public:
		std::uint32_t compressed_len_;	// 0x000C
		std::uint32_t len_;				// 0x0010
	private:
		char pad_0014[0x0004];			// 0x0014
	public:
		const char* buffer_;			// 0x0018
	};
	ENGINE_ASSERT_SZ(RawFile, 0x0020);
}

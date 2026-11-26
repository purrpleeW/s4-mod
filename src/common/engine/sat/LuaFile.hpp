#pragma once
#include "engine/engine_common.hpp"

namespace sat {
	class LuaFile {
	public:
		std::uint64_t hash_;			// 0x0000
	private:
		char pad_0008[0x0033];			// 0x0008
	public:
		bool is_compressed_;			// 0x003B
		int len_;						// 0x003C
		const std::uint8_t* buffer_;	// 0x0040
	private:
		char pad_0048[0x0020];			// 0x0048
	};
	ENGINE_ASSERT_SZ(LuaFile, 0x0068);
}

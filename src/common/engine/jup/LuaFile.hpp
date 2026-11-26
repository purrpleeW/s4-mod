#pragma once
#include "engine/engine_common.hpp"

namespace jup {
	class LuaFile {
	public:
		std::uint64_t hash_;			// 0x0000
		int len_;						// 0x0008
		std::uint8_t stripping_type_;	// 0x000C
		bool is_compressed_;			// 0x000D
	private:
		char pad_000E[0x0002];			// 0x000E
	public:
		const std::uint8_t* buffer_;	// 0x0010
	};
	ENGINE_ASSERT_SZ(LuaFile, 0x0018);
}

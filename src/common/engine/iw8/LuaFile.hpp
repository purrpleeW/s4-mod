#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class LuaFile {
	public:
		const char* name_;				// 0x0000
		int len_;						// 0x0008
		std::uint8_t stripping_type_;	// 0x000C
	private:
		char pad_000D[0x0003];			// 0x000D
	public:
		std::uint8_t* buffer_;			// 0x0010
	};
	ENGINE_ASSERT_SZ(LuaFile, 0x0018);
}

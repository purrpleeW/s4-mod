#pragma once
#include "engine/engine_common.hpp"

namespace t10 {
	class LuaFile {
	public:
		std::uint64_t hash_;			// 0x0000
	private:
		char pad_0008[0x002C];
	public:
		int len_;						// 0x0034
		std::uint8_t stripping_type_;	// 0x0038
		bool is_compressed_;			// 0x0039
	private:
		char pad_003A[0x0006];			// 0x003A
	public:
		const std::uint8_t* buffer_;	// 0x0040
	private:
		char pad_0048[0x0010];			// 0x0048
	};
	ENGINE_ASSERT_SZ(LuaFile, 0x0058);
}

#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class ScriptFile {
	public:
		const char* name_;			// 0x0000
		int compressed_len_;		// 0x0008
		int len_;					// 0x000C
		int bytecode_len_;			// 0x0010
	private:
		char pad_0014[0x0004];		// 0x0014
	public:
		const char* buffer_;		// 0x0018
		std::uint8_t* bytecode_;	// 0x0020
	};
	ENGINE_ASSERT_SZ(ScriptFile, 0x0028);
}

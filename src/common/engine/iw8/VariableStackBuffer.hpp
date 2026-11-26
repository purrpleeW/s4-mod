#pragma once
#include "engine/engine_common.hpp"
#include "engine/iw8/ScriptCodePos.hpp"

namespace iw8 {
	class VariableStackBuffer {
	public:
		ScriptCodePos code_pos_;	// 0x0000
		std::uint16_t size_;		// 0x0008
		std::uint16_t buf_len_;		// 0x000A
		std::uint32_t local_id_;	// 0x000C
		std::uint8_t time_;			// 0x0010
		char buf[1];				// 0x0011
	private:
		char pad_0012[0x0006];		// 0x0012
	};
	ENGINE_ASSERT_SZ(VariableStackBuffer, 0x0018);
}

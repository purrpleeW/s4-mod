#pragma once
#include "engine/engine_common.hpp"
#include "engine/t9/GSC_OBJ.hpp"

namespace t9 {
	class ScriptParseTree {
	public:
		std::uint64_t name_;	// 0x0000
		GSC_OBJ* buffer_;		// 0x0008
		int len_;				// 0x0010
	private:
		char pad_0014[0x0004];	// 0x0014
	};
}

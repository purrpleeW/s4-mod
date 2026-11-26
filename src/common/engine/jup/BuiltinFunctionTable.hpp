#pragma once
#include "engine/engine_common.hpp"
#include "engine/jup/BuiltinFunctionDef.hpp"

namespace jup {
	class BuiltinFunctionTable {
	public:
		const BuiltinFunctionDef* functions_;	// 0x0000
		std::size_t count_;						// 0x0008
	};
	ENGINE_ASSERT_SZ(BuiltinFunctionTable, 0x0010);
}

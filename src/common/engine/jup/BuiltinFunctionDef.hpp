#pragma once
#include "engine/engine_common.hpp"

namespace jup {
	class BuiltinFunctionDef {
	public:
		XHash name_;							// 0x0000
		void (*action_func_)(scrContext_t*);	// 0x0008
	private:
		char pad_0010[0x0008];					// 0x0010
	};
	ENGINE_ASSERT_SZ(BuiltinFunctionDef, 0x0018);
}

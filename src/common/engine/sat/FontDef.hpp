#pragma once
#include "engine/engine_common.hpp"

namespace sat {
	class FontDef {
	public:
		sat::XHash hash_;		// 0x0000
	private:
		char pad_0008[0x0070];	// 0x0008
	};
	ENGINE_ASSERT_SZ(FontDef, 0x0078);
}

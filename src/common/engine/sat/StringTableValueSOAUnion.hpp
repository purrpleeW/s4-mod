#pragma once
#include "engine/engine_common.hpp"

namespace sat {
	union StringTableValueSOAUnion {
		const char** strings_;
		float* floats_;
		int* ints_;
		bool* bools_;
		XHash* xhashes_;
		std::uint64_t* uint64s_;
	};
	ENGINE_ASSERT_SZ(StringTableValueSOAUnion, 0x0008);
}

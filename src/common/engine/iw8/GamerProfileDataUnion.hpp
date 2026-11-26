#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	union GamerProfileDataUnion {
		std::uint8_t byte_val_;		// 0x0000
		bool bool_val_;				// 0x0000
		std::int16_t short_val_;	// 0x0000
		int int_val_;				// 0x0000
		float float_val_;			// 0x0000
		const char* string_val_;	// 0x0000
	};
	ENGINE_ASSERT_SZ(GamerProfileDataUnion, 0x0008);
}

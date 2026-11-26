#pragma once
#include "engine/engine_common.hpp"

namespace t9 {
	class netipv4_t {
	public:
		std::uint8_t a;	// 0x0000
		std::uint8_t b;	// 0x0001
		std::uint8_t c;	// 0x0002
		std::uint8_t d;	// 0x0003
	};
	ENGINE_ASSERT_SZ(netipv4_t, 0x0004);
}

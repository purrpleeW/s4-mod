#pragma once
#include "engine/engine_common.hpp"

namespace t9 {
	class KeyState {
	public:
		int down_;			// 0x0000
		int repeats_;		// 0x0004
		int binding_;		// 0x0008
	private:
		char pad_000C[4];	// 0x000C
	};
	ENGINE_ASSERT_SZ(KeyState, 0x0010);
}

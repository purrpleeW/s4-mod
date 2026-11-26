#pragma once
#include "engine/engine_common.hpp"

namespace iw8 {
	class KeyState {
	public:
		std::uint8_t repeats_;	// 0x0000
		bool is_super_user_;	// 0x0001
		bool down_;				// 0x0002
	private:
		char pad_0003[0x0001];	// 0x0003
	};
	ENGINE_ASSERT_SZ(KeyState, 0x0004);
}
